#include "json_reader.h"

/**
 * Allocates a fresh unused token from the token pool.
 */
JsonToken *jsmn_alloc_token(JsonParser *parser, JsonToken *tokens, const size_t num_tokens) {
    if (parser->nextTokenIndex >= num_tokens) {
        return nullptr;
    }
    JsonToken *token = &tokens[parser->nextTokenIndex++];
    token->start = token->end = -1;
    token->childrenCount      = 0;
    return token;
}

/**
 * Fills token type and boundaries.
 */
void JsonToken_Init(JsonToken *token, const JsonType type, const size_t start, const size_t end) {
    token->type          = type;
    token->start         = start;
    token->end           = end;
    token->childrenCount = 0;
}

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
void JsonParser_Init(JsonParser *parser) {
    parser->offset           = 0;
    parser->nextTokenIndex   = 0;
    parser->parentTokenIndex = -1;
}

/**
 * Fills next available token with JSON primitive.
 */
JsonParseErr JsonParser_ParsePrimitive(
    JsonParser *parser, const char *s, const size_t len, JsonToken *tokens, const size_t tokensLen
) {
    JsonToken   *token;
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
                goto found;
            default:
                /* to quiet a warning from gcc*/
                break;
        }
        if (s[parser->offset] < 32 || s[parser->offset] >= 127) {
            parser->offset = start;
            return JSON_PARSE_ERROR_INVALID;
        }
    }

    /* In strict mode primitive must be followed by a comma/object/array */
    parser->offset = start;
    return JSON_PARSE_ERROR_PARTIAL;

found:
    if (tokens == nullptr) {
        parser->offset--;
        return 0;
    }
    token = jsmn_alloc_token(parser, tokens, tokensLen);
    if (token == nullptr) {
        parser->offset = start;
        return JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED;
    }
    JsonToken_Init(token, JSON_TYPE_PRIMITIVE, start, parser->offset);

    parser->offset--;
    return 0;
}

/**
 * Fills next token with JSON string.
 */
JsonParseErr JsonParser_ParseString(
    JsonParser *parser, const char *js, const size_t len, JsonToken *tokens, const size_t tokensLen
) {
    const size_t start = parser->offset;

    /* Skip starting quote */
    parser->offset++;

    for (; parser->offset < len && js[parser->offset] != '\0'; parser->offset++) {
        char c = js[parser->offset];

        /* Quote: end of string */
        if (c == '\"') {
            if (tokens == nullptr) {
                return 0;
            }
            JsonToken *token = jsmn_alloc_token(parser, tokens, tokensLen);
            if (token == nullptr) {
                parser->offset = start;
                return JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED;
            }
            JsonToken_Init(token, JSON_TYPE_STRING, start + 1, parser->offset);

            return 0;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && parser->offset + 1 < len) {
            int i;
            parser->offset++;
            switch (js[parser->offset]) {
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
                    for (i = 0; i < 4 && parser->offset < len && js[parser->offset] != '\0'; i++) {
                        /* If it isn't a hex character we have an error */
                        if (!((js[parser->offset] >= 48 && js[parser->offset] <= 57) || /* 0-9 */
                              (js[parser->offset] >= 65 && js[parser->offset] <= 70) || /* A-F */
                              (js[parser->offset] >= 97 && js[parser->offset] <= 102))) {
                            /* a-f */
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
JsonParseErr JsonParser_Parse(
    JsonParser *parser, const char *s, const size_t jsonLen, JsonToken *tokens, const size_t tokensLen
) {
    int          r;
    JsonToken   *token;
    unsigned int count = parser->nextTokenIndex;

    for (; parser->offset < jsonLen && s[parser->offset] != '\0'; parser->offset++) {
        JsonType type;

        char c = s[parser->offset];
        switch (c) {
            case '{':
            case '[':
                count++;
                if (tokens == nullptr) {
                    break;
                }
                token = jsmn_alloc_token(parser, tokens, tokensLen);
                if (token == nullptr) {
                    return JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED;
                }
                if (parser->parentTokenIndex != -1) {
                    JsonToken *t = &tokens[parser->parentTokenIndex];

                    /* In strict mode an object or array can't become a key */
                    if (t->type == JSON_TYPE_OBJECT) {
                        return JSON_PARSE_ERROR_INVALID;
                    }

                    t->childrenCount++;
                }
                token->type              = (c == '{' ? JSON_TYPE_OBJECT : JSON_TYPE_ARRAY);
                token->start             = parser->offset;
                parser->parentTokenIndex = (int)parser->nextTokenIndex - 1;
                break;
            case '}':
            case ']':
                if (tokens == nullptr) {
                    break;
                }
                type = (c == '}' ? JSON_TYPE_OBJECT : JSON_TYPE_ARRAY);

                int i = (int)parser->nextTokenIndex - 1;
                for (; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return JSON_PARSE_ERROR_INVALID;
                        }
                        parser->parentTokenIndex = -1;
                        token->end               = parser->offset + 1;
                        break;
                    }
                }
                /* Error if unmatched closing bracket */
                if (i == -1) {
                    return JSON_PARSE_ERROR_INVALID;
                }
                for (; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        parser->parentTokenIndex = i;
                        break;
                    }
                }

                break;
            case '\"':
                r = JsonParser_ParseString(parser, s, jsonLen, tokens, tokensLen);
                if (r < 0) {
                    return r;
                }
                count++;
                if (parser->parentTokenIndex != -1 && tokens != nullptr) {
                    tokens[parser->parentTokenIndex].childrenCount++;
                }
                break;
            case '\t':
            case '\r':
            case '\n':
            case ' ':
                break;
            case ':':
                parser->parentTokenIndex = (int)parser->nextTokenIndex - 1;
                break;
            case ',':
                if (tokens != nullptr && parser->parentTokenIndex != -1 &&
                    tokens[parser->parentTokenIndex].type != JSON_TYPE_ARRAY &&
                    tokens[parser->parentTokenIndex].type != JSON_TYPE_OBJECT) {
                    for (int i = (int)parser->nextTokenIndex - 1; i >= 0; i--) {
                        if (tokens[i].type == JSON_TYPE_ARRAY || tokens[i].type == JSON_TYPE_OBJECT) {
                            if (tokens[i].start != -1 && tokens[i].end == -1) {
                                parser->parentTokenIndex = i;
                                break;
                            }
                        }
                    }
                }
                break;

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
            case 'n':
                /* And they must not be keys of the object */
                if (tokens != nullptr && parser->parentTokenIndex != -1) {
                    const JsonToken *t = &tokens[parser->parentTokenIndex];
                    if (t->type == JSON_TYPE_OBJECT || (t->type == JSON_TYPE_STRING && t->childrenCount != 0)) {
                        return JSON_PARSE_ERROR_INVALID;
                    }
                }

                r = JsonParser_ParsePrimitive(parser, s, jsonLen, tokens, tokensLen);
                if (r < 0) {
                    return r;
                }
                count++;
                if (parser->parentTokenIndex != -1 && tokens != nullptr) {
                    tokens[parser->parentTokenIndex].childrenCount++;
                }
                break;

            /* Unexpected char in strict mode */
            default:
                return JSON_PARSE_ERROR_INVALID;
        }
    }

    if (tokens != nullptr) {
        for (int i = parser->nextTokenIndex - 1; i >= 0; i--) {
            /* Unmatched opened object or array */
            if (tokens[i].start != -1 && tokens[i].end == -1) {
                return JSON_PARSE_ERROR_PARTIAL;
            }
        }
    }

    return count;
}
