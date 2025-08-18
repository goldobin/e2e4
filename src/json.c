#include "json.h"

#include <assert.h>

static bool isVisibleChar(const char ch) { return ch >= 32 && ch <= 126; /* ' ' - ~ */ }

static bool isHexChar(const char ch) {
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
static JsonParseResult JsonParsePrimitive(JsonTokens *dst, const char *s, const size_t len, size_t offset) {
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
                    .type     = JSON_TOKEN_TYPE_PRIMITIVE,
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
static JsonParseResult JsonParseString(JsonTokens *dst, const char *s, const size_t len, size_t offset) {
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
                .type     = JSON_TOKEN_TYPE_STRING,
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
                    if (parentT->type == JSON_TOKEN_TYPE_OBJECT) {
                        return (JsonParseResult){

                            .err  = JSON_PARSE_ERROR_INVALID,
                            .read = offset,
                        };
                    }

                    parentT->childrenCount++;
                }

                *t = (JsonToken){
                    .type     = (c == '{' ? JSON_TOKEN_TYPE_OBJECT : JSON_TOKEN_TYPE_ARRAY),
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

                const JsonTokenType type = (c == '}' ? JSON_TOKEN_TYPE_OBJECT : JSON_TOKEN_TYPE_ARRAY);
                int                 i    = (int)dst->len - 1;
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
                    if (parenT->type == JSON_TOKEN_TYPE_ARRAY || parenT->type == JSON_TOKEN_TYPE_OBJECT) {
                        break;
                    }
                    for (int i = (int)dst->len - 1; i >= 0; i--) {
                        const auto t = JsonTokens_At(dst, i);
                        if (t->type == JSON_TOKEN_TYPE_ARRAY || t->type == JSON_TOKEN_TYPE_OBJECT) {
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
                if (t->type == JSON_TOKEN_TYPE_OBJECT || (t->type == JSON_TOKEN_TYPE_STRING && t->childrenCount != 0)) {
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

JsonNode *JsonStack_Push(JsonStack *s) {
    assert(s != nullptr);
    assert(s->cap - s->len > 0);

    return &s->arr[s->len++];
}

JsonNode *JsonStack_Top(const JsonStack *s) {
    assert(s != nullptr);
    assert(s->len > 0);
    return &s->arr[s->len - 1];
}

JsonNode JsonStack_Pull(JsonStack *s) {
    assert(s != nullptr);
    assert(s->len > 0);
    return s->arr[--s->len];
}

size_t CharSlice_WriteJsonStart(CharSlice *dst, JsonStack *s, const char bracket) {
    assert(dst != nullptr);
    assert(s != nullptr);

    JsonNodeType t = JSON_NODE_TYPE_UNDEFINED;
    switch (bracket) {
        case '{':
            t = JSON_NODE_TYPE_OBJECT;
            break;
        case '[':
            t = JSON_NODE_TYPE_ARRAY;
            break;
        default:
            assert(false);
    }

    if (s->len > 0) {
        const auto n = JsonStack_Top(s);
        assert(n != nullptr);
        assert(n->type == JSON_NODE_TYPE_FIELD);
    }

    size_t written = 0;
    written += CharSlice_WriteChar(dst, bracket);
    const auto next = JsonStack_Push(s);
    assert(next != nullptr);

    *next = (JsonNode){.type = t};
    return written;
}

size_t CharSlice_WriteJsonEnd(CharSlice *dst, JsonStack *s) {
    assert(dst != nullptr);
    assert(s != nullptr);
    assert(s->len > 0);

    const auto pulled = JsonStack_Pull(s);

    size_t written = 0;
    switch (pulled.type) {
        case JSON_NODE_TYPE_OBJECT:
            written += CharSlice_WriteChar(dst, '}');
            break;
        case JSON_NODE_TYPE_ARRAY:
            written += CharSlice_WriteChar(dst, ']');
            break;
        default:
            assert(false);
    }

    if (s->len > 0 && JsonStack_Top(s)->type == JSON_NODE_TYPE_FIELD) {
        JsonStack_Pull(s);
    }

    return written;
}

size_t CharSlice_WriteJsonKey(CharSlice *dst, JsonStack *s, CharSlice key) {
    assert(dst != nullptr);
    assert(s != nullptr);
    assert(CharSlice_IsValid(&key));

    const auto parent = JsonStack_Top(s);
    assert(parent != nullptr);
    assert(parent->type == JSON_NODE_TYPE_OBJECT);

    size_t written = 0;

    if (parent->needsComma) {
        written += CharSlice_WriteChar(dst, ',');
    }
    parent->needsComma = true;

    const auto next = JsonStack_Push(s);
    assert(next != nullptr);

    *next = (JsonNode){.type = JSON_NODE_TYPE_FIELD};

    written += CharSlice_WriteChar(dst, '\"');
    written += CharSlice_Write(dst, key);
    written += CharSlice_WriteChar(dst, '\"');
    written += CharSlice_WriteChar(dst, ':');

    return written;
}

size_t CharSlice_WritePrimitivePreamble(CharSlice *dst, JsonStack *s) {
    assert(dst != nullptr);
    assert(s != nullptr);
    assert(s->len > 0);

    size_t     written = 0;
    const auto parent  = JsonStack_Top(s);
    switch (parent->type) {
        case JSON_NODE_TYPE_FIELD:
            JsonStack_Pull(s);
            parent->needsComma = true;
            break;
        case JSON_NODE_TYPE_ARRAY:
            if (parent->needsComma) {
                written += CharSlice_WriteChar(dst, ',');
            }
            parent->needsComma = true;
            break;
        default:
            assert(false);
    }

    return written;
}

size_t CharSlice_WriteJsonValue(CharSlice *dst, JsonStack *s, const bool isString, const CharSlice value) {
    assert(dst != nullptr);
    assert(s != nullptr);
    assert(CharSlice_IsValid(&value));

    size_t written = 0;
    written += CharSlice_WritePrimitivePreamble(dst, s);
    if (isString) {
        written += CharSlice_WriteChar(dst, '\"');
        for (size_t i = 0; i < value.len; ++i) {
            const auto ch = CharSlice_At(&value, i);
            switch (ch) {
                case '\"':
                    written += CharSlice_Write(dst, CHAR_SLICE("\\\""));
                    break;
                case '\\':
                    written += CharSlice_Write(dst, CHAR_SLICE("\\\\"));
                case '\b':
                    written += CharSlice_Write(dst, CHAR_SLICE("\\\b"));
                case '\f':
                    written += CharSlice_Write(dst, CHAR_SLICE("\\\f"));
                case '\n':
                    written += CharSlice_Write(dst, CHAR_SLICE("\\\n"));
                case '\r':
                    written += CharSlice_Write(dst, CHAR_SLICE("\\\r"));
                case '\t':
                    written += CharSlice_Write(dst, CHAR_SLICE("\\\t"));
                default:
                    written += CharSlice_WriteChar(dst, ch);
            }
        }
        written += CharSlice_WriteChar(dst, '\"');
        return written;
    }

    assert(value.len > 0);
    const auto firstCh = CharSlice_At(&value, 0);
    switch (firstCh) {
        case 'n':
            assert(CharSlice_Cmp(value, CHAR_SLICE("null")) == 0);
            return CharSlice_Write(dst, CHAR_SLICE("null"));
        case 't':
            assert(CharSlice_Cmp(value, CHAR_SLICE("true")) == 0);
            return CharSlice_Write(dst, CHAR_SLICE("true"));
        case 'f':
            assert(CharSlice_Cmp(value, CHAR_SLICE("false")) == 0);
            return CharSlice_Write(dst, CHAR_SLICE("false"));
        default:
            if (!(firstCh == '-' || (firstCh >= '0' && firstCh <= '9'))) {
                assert(false);
            }
            written += CharSlice_WriteChar(dst, firstCh);
            bool wasPoint = false;
            for (size_t i = 1; i < value.len; ++i) {
                const auto ch = CharSlice_At(&value, i);
                if (ch == '.') {
                    if (wasPoint) {
                        assert(false);
                    }
                    wasPoint = true;
                    written += CharSlice_WriteChar(dst, '.');
                } else if (ch >= '0' && ch <= '9') {
                    written += CharSlice_WriteChar(dst, ch);
                } else {
                    assert(false);
                }
            }
            return written;
    }
}
