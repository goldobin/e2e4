#include "json_reader.h"

#include <assert.h>

bool isVisibleChar(const char ch) { return ch >= 32 && ch <= 126; /* ' ' - ~ */ }

bool isHexChar(const char ch) {
    return (ch >= 48 && ch <= 57) /* 0-9 */ || (ch >= 65 && ch <= 70) /* A-F */ || (ch >= 97 && ch <= 102); /* a-f */
}

/**
 * Fills token type and boundaries.
 */
void JsonToken_Init(JsonToken *token, const JsonType type, const size_t start, const int end) {
    token->type          = type;
    token->start         = start;
    token->end           = end;
    token->childrenCount = 0;
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

    JsonToken_Init(t, JSON_TYPE_UNDEFINED, 0, -1);
    return t;
}

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
void JsonParser_Init(JsonParser *parser) {
    parser->offset           = 0;
    parser->parentTokenIndex = -1;
}

/**
 * Fills next available token with JSON primitive.
 */
JsonParseErr JsonParser_ParsePrimitive(JsonParser *parser, JsonTokens *dst, const char *s, const size_t len) {
    assert(parser != nullptr);
    assert(dst != nullptr);

    const size_t start = parser->offset;

    for (; parser->offset < len && s[parser->offset] != '\0'; parser->offset++) {
        switch (s[parser->offset]) {
            /* In strict mode primitive must be followed by "," or "}" or "]" */
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
                    parser->offset = start;
                    return JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED;
                }
                JsonToken_Init(t, JSON_TYPE_PRIMITIVE, start, (int)parser->offset);

                parser->offset--;
                return JSON_PARSE_ERROR_OK;
            default:
                /* to quiet a warning from gcc*/
                break;
        }
        const auto ch = s[parser->offset];
        if (!isVisibleChar(ch)) {
            parser->offset = start;
            return JSON_PARSE_ERROR_INVALID;
        }
    }

    /* In strict mode primitive must be followed by a comma/object/array */
    parser->offset = start;
    return JSON_PARSE_ERROR_PARTIAL;
}

/**
 * Fills next token with JSON string.
 */
JsonParseErr JsonParser_ParseString(JsonParser *parser, JsonTokens *dst, const char *s, const size_t len) {
    assert(parser != nullptr);
    assert(dst != nullptr);

    const auto start = parser->offset;
    /* Skip starting quote */
    parser->offset++;

    for (; parser->offset < len && s[parser->offset] != '\0'; parser->offset++) {
        const char c = s[parser->offset];

        /* Quote: end of string */
        if (c == '\"') {
            const auto t = JsonTokens_Grow(dst, 1);
            if (t == nullptr) {
                parser->offset = start;
                return JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED;
            }

            JsonToken_Init(t, JSON_TYPE_STRING, start + 1, (int)parser->offset);
            return JSON_PARSE_ERROR_OK;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && parser->offset + 1 < len) {
            int i;
            parser->offset++;
            switch (s[parser->offset]) {
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
                    parser->offset++;
                    for (i = 0; i < 4 && parser->offset < len && s[parser->offset] != '\0'; i++) {
                        /* If it isn't a hex character we have an error */
                        const auto ch = s[parser->offset];
                        if (!isHexChar(ch)) {
                            parser->offset = start;
                            return JSON_PARSE_ERROR_INVALID;
                        }
                        parser->offset++;
                    }
                    parser->offset--;
                    break;
                /* Unexpected symbol */
                default:
                    parser->offset = start;
                    return JSON_PARSE_ERROR_INVALID;
            }
        }
    }
    parser->offset = start;
    return JSON_PARSE_ERROR_PARTIAL;
}

/**
 * Parse JSON string and fill tokens.
 */
JsonParseErr JsonParser_Parse(JsonParser *parser, JsonTokens *dst, const char *s, const size_t len) {
    assert(parser != nullptr);
    assert(dst != nullptr);

    // int r;
    for (; parser->offset < len && s[parser->offset] != '\0'; parser->offset++) {
        const char c = s[parser->offset];
        switch (c) {
            case '{':
            case '[': {
                const auto t = JsonTokens_Grow(dst, 1);
                if (t == nullptr) {
                    return JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED;
                }

                if (parser->parentTokenIndex != -1) {
                    JsonToken *parentT = JsonTokens_At(dst, parser->parentTokenIndex);

                    /* In strict mode an object or array can't become a key */
                    if (parentT->type == JSON_TYPE_OBJECT) {
                        return JSON_PARSE_ERROR_INVALID;
                    }

                    parentT->childrenCount++;
                }

                JsonToken_Init(t, (c == '{' ? JSON_TYPE_OBJECT : JSON_TYPE_ARRAY), parser->offset, -1);
                parser->parentTokenIndex = (int)dst->len - 1;
                break;
            }
            case '}':
            case ']': {
                const JsonType type = (c == '}' ? JSON_TYPE_OBJECT : JSON_TYPE_ARRAY);
                int            i    = (int)dst->len - 1;
                for (; i >= 0; i--) {
                    const auto t = JsonTokens_At(dst, i);
                    if (t->end == -1) {
                        if (t->type != type) {
                            return JSON_PARSE_ERROR_INVALID;
                        }
                        parser->parentTokenIndex = -1;
                        t->end                   = (int)parser->offset + 1;
                        break;
                    }
                }
                /* Error if unmatched closing bracket */
                if (i == -1) {
                    return JSON_PARSE_ERROR_INVALID;
                }
                for (; i >= 0; i--) {
                    const auto t = JsonTokens_At(dst, i);
                    if (t->end == -1) {
                        parser->parentTokenIndex = i;
                        break;
                    }
                }
                break;
            }
            case '\"': {
                const auto err = JsonParser_ParseString(parser, dst, s, len);
                if (err != JSON_PARSE_ERROR_OK) {
                    return err;
                }
                if (parser->parentTokenIndex != -1) {
                    const auto t = JsonTokens_At(dst, parser->parentTokenIndex);
                    t->childrenCount++;
                }
                break;
            }
            case '\t':
            case '\r':
            case '\n':
            case ' ':
                break;
            case ':':
                parser->parentTokenIndex = (int)dst->len - 1;
                break;
            case ',': {
                if (parser->parentTokenIndex != -1) {
                    const auto parenT = JsonTokens_At(dst, parser->parentTokenIndex);
                    if (parenT->type == JSON_TYPE_ARRAY || parenT->type == JSON_TYPE_OBJECT) {
                        break;
                    }
                    for (int i = (int)dst->len - 1; i >= 0; i--) {
                        const auto t = JsonTokens_At(dst, i);
                        if (t->type == JSON_TYPE_ARRAY || t->type == JSON_TYPE_OBJECT) {
                            if (t->end == -1) {
                                parser->parentTokenIndex = i;
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
                /* And they must not be keys of the object */
                if (parser->parentTokenIndex != -1) {
                    const auto t = JsonTokens_At(dst, parser->parentTokenIndex);
                    if (t->type == JSON_TYPE_OBJECT || (t->type == JSON_TYPE_STRING && t->childrenCount != 0)) {
                        return JSON_PARSE_ERROR_INVALID;
                    }
                }

                const auto err = JsonParser_ParsePrimitive(parser, dst, s, len);
                if (err != JSON_PARSE_ERROR_OK) {
                    return err;
                }
                if (parser->parentTokenIndex != -1) {
                    const auto t = JsonTokens_At(dst, parser->parentTokenIndex);
                    t->childrenCount++;
                }
                break;
            }

            /* Unexpected char in strict mode */
            default:
                return JSON_PARSE_ERROR_INVALID;
        }
    }

    for (int i = (int)dst->len - 1; i >= 0; i--) {
        /* Unmatched opened object or array */
        const auto t = JsonTokens_At(dst, i);
        if (t->end == -1) {
            return JSON_PARSE_ERROR_PARTIAL;
        }
    }

    return JSON_PARSE_ERROR_OK;
}
