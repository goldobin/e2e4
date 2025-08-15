#include "json_reader.h"

#include <assert.h>

bool isVisibleChar(const char ch) { return ch >= 32 && ch <= 126; /* ' ' - ~ */ }

bool isHexChar(const char ch) {
    return (ch >= 48 && ch <= 57) /* 0-9 */ || (ch >= 65 && ch <= 70) /* A-F */ || (ch >= 97 && ch <= 102); /* a-f */
}

JsonToken *JsonTokens_At(const JsonTokens *ts, const size_t index) {
    assert(ts != nullptr);
    assert(index < ts->len);
    return &ts->arr[index];
}

/**
 * Allocates a fresh unused token from the token pool.
 */
JsonToken *JsonTokens_Grow(JsonTokens *ts, const size_t len) {
    if (ts->len + len > ts->cap) {
        return nullptr;
    }

    JsonToken *t = &ts->arr[ts->len];
    ts->len += len;

    return t;
}

/**
 * Fills next available token with JSON primitive.
 */
JsonParseResult JsonParsePrimitive(JsonTokens *dst, const char *s, const size_t len, size_t offset) {
    assert(dst != nullptr);

    const size_t start = offset;
    for (; offset < len && s[offset] != '\0'; offset++) {
        const char c = s[offset];
        switch (c) {
            case ':':
            case '\t':
            case '\r':
            case '\n':
            case ' ':
            case ',':
            case ']':
            case '}':
                const auto t = JsonTokens_Grow(dst, 1);
                if (t == nullptr) {
                    return (JsonParseResult){.err = JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED};
                }

                *t = (JsonToken){
                    .type     = JSON_TYPE_PRIMITIVE,
                    .offset   = start,
                    .len      = offset - start,
                    .finished = true,
                };

                offset--;
                return (JsonParseResult){
                    .read = offset - start,
                };
            default:
                /* to quiet a warning from gcc*/
                break;
        }
        if (!isVisibleChar(c)) {
            return (JsonParseResult){.err = JSON_PARSE_ERROR_INVALID};
        }
    }

    /* In strict mode primitive must be followed by a comma/object/array */
    return (JsonParseResult){.err = JSON_PARSE_ERROR_PARTIAL};
}

/**
 * Fills next token with JSON string.
 */
JsonParseResult JsonParseString(JsonTokens *dst, const char *s, const size_t len, size_t offset) {
    assert(dst != nullptr);

    const auto start = offset;
    /* Skip starting quote */
    offset++;

    for (; offset < len && s[offset] != '\0'; offset++) {
        const char c = s[offset];

        /* Quote: end of string */
        if (c == '\"') {
            const auto t = JsonTokens_Grow(dst, 1);
            if (t == nullptr) {
                return (JsonParseResult){.err = JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED};
            }

            *t = (JsonToken){
                .type     = JSON_TYPE_STRING,
                .offset   = start + 1,
                .len      = offset - (start + 1),
                .finished = true,
            };

            return (JsonParseResult){
                .err  = JSON_PARSE_ERROR_OK,
                .read = offset - start,
            };
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && offset + 1 < len) {
            offset++;
            switch (s[offset]) {
                /* Allowed escaped symbols */
                case '\"':
                case '/':
                case '\\':
                case 'b':
                case 'f':
                case 'r':
                case 'n':
                case 't':
                    break;
                /* Allows escaped symbol \uXXXX */
                case 'u':
                    offset++;
                    for (size_t i = 0; i < 4 && offset < len && s[offset] != '\0'; i++) {
                        /* If it isn't a hex character we have an error */
                        const auto ch = s[offset];
                        if (!isHexChar(ch)) {
                            return (JsonParseResult){.err = JSON_PARSE_ERROR_INVALID};
                        }
                        offset++;
                    }
                    offset--;
                    break;
                /* Unexpected symbol */
                default:
                    return (JsonParseResult){.err = JSON_PARSE_ERROR_INVALID};
            }
        }
    }

    return (JsonParseResult){.err = JSON_PARSE_ERROR_PARTIAL};
}

/**
 * Parse JSON string and fill tokens.
 */
JsonParseResult JsonParse(JsonTokens *dst, const char *s, const size_t len) {
    assert(dst != nullptr);

    int    parentTokenIndex = -1;
    size_t offset           = 0;
    for (; offset < len && s[offset] != '\0'; offset++) {
        const char c = s[offset];
        switch (c) {
            case '{':
            case '[': {
                const auto t = JsonTokens_Grow(dst, 1);
                if (t == nullptr) {
                    return (JsonParseResult){
                        .err  = JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED,
                        .read = offset,
                    };
                }

                if (parentTokenIndex != -1) {
                    JsonToken *parentT = JsonTokens_At(dst, parentTokenIndex);

                    /* In strict mode an object or array can't become a key */
                    if (parentT->type == JSON_TYPE_OBJECT) {
                        return (JsonParseResult){

                            .err  = JSON_PARSE_ERROR_INVALID,
                            .read = offset,
                        };
                    }

                    parentT->childrenCount++;
                }

                *t = (JsonToken){
                    .type     = (c == '{' ? JSON_TYPE_OBJECT : JSON_TYPE_ARRAY),
                    .offset   = offset,
                    .len      = 0,
                    .finished = false,
                };

                parentTokenIndex = (int)dst->len - 1;
                break;
            }
            case '}':
            case ']': {
                if (parentTokenIndex == -1) {
                    return (JsonParseResult){.err = JSON_PARSE_ERROR_INVALID};
                }

                const JsonType type = (c == '}' ? JSON_TYPE_OBJECT : JSON_TYPE_ARRAY);
                int            i    = (int)dst->len - 1;
                for (; i >= 0; i--) {
                    const auto t = JsonTokens_At(dst, i);
                    if (t->finished) {
                        continue;
                    }
                    if (t->type != type) {
                        return (JsonParseResult){
                            .err  = JSON_PARSE_ERROR_INVALID,
                            .read = offset,
                        };
                    }
                    parentTokenIndex = -1;
                    t->len           = offset - t->offset + 1;
                    t->finished      = true;
                    break;
                }
                /* Error if unmatched closing bracket */
                if (i == -1) {
                    return (JsonParseResult){
                        .err  = JSON_PARSE_ERROR_INVALID,
                        .read = offset,
                    };
                }
                for (; i >= 0; i--) {
                    const auto t = JsonTokens_At(dst, i);
                    if (!t->finished) {
                        parentTokenIndex = i;
                        break;
                    }
                }

                if (parentTokenIndex == -1) {
                    offset++;
                    goto finished;
                }

                break;
            }
            case '\"': {
                if (parentTokenIndex == -1) {
                    return (JsonParseResult){.err = JSON_PARSE_ERROR_INVALID};
                }

                const auto r = JsonParseString(dst, s, len, offset);
                if (r.err != JSON_PARSE_ERROR_OK) {
                    return (JsonParseResult){
                        .err  = r.err,
                        .read = offset,
                    };
                }
                offset += r.read;
                const auto parentT = JsonTokens_At(dst, parentTokenIndex);
                parentT->childrenCount++;

                break;
            }
            case '\t':
            case '\r':
            case '\n':
            case ' ':
                break;
            case ':': {
                if (parentTokenIndex == -1) {
                    return (JsonParseResult){.err = JSON_PARSE_ERROR_INVALID};
                }

                parentTokenIndex = (int)dst->len - 1;
                break;
            }
            case ',': {
                if (parentTokenIndex != -1) {
                    const auto parenT = JsonTokens_At(dst, parentTokenIndex);
                    if (parenT->type == JSON_TYPE_ARRAY || parenT->type == JSON_TYPE_OBJECT) {
                        break;
                    }
                    for (int i = (int)dst->len - 1; i >= 0; i--) {
                        const auto t = JsonTokens_At(dst, i);
                        if (t->type == JSON_TYPE_ARRAY || t->type == JSON_TYPE_OBJECT) {
                            if (!t->finished) {
                                parentTokenIndex = i;
                                break;
                            }
                        }
                    }
                }
                break;
            }

            /* In strict mode primitives are: numbers and booleans */
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 't':
            case 'f':
            case 'n': {
                if (parentTokenIndex == -1) {
                    return (JsonParseResult){.err = JSON_PARSE_ERROR_INVALID};
                }

                const auto t = JsonTokens_At(dst, parentTokenIndex);
                if (t->type == JSON_TYPE_OBJECT || (t->type == JSON_TYPE_STRING && t->childrenCount != 0)) {
                    return (JsonParseResult){
                        .err  = JSON_PARSE_ERROR_INVALID,
                        .read = offset,
                    };
                }

                const auto r = JsonParsePrimitive(dst, s, len, offset);
                if (r.err != JSON_PARSE_ERROR_OK) {
                    return (JsonParseResult){
                        .err  = r.err,
                        .read = offset,
                    };
                }
                offset += r.read;

                const auto parenT = JsonTokens_At(dst, parentTokenIndex);
                parenT->childrenCount++;

                break;
            }

            /* Unexpected char in strict mode */
            default:
                return (JsonParseResult){
                    .err  = JSON_PARSE_ERROR_INVALID,
                    .read = offset,
                };
        }
    }

finished:
    for (int i = (int)dst->len - 1; i >= 0; i--) {
        /* Unmatched opened object or array */
        const auto t = JsonTokens_At(dst, i);
        if (!t->finished) {
            return (JsonParseResult){
                .err  = JSON_PARSE_ERROR_PARTIAL,
                .read = offset,
            };
        }
    }

    return (JsonParseResult){.read = offset};
}
