#include "json.h"

#include <assert.h>
#include <string.h>

static bool isVisibleChar(const char ch) { return ch >= 32 && ch <= 126; /* ' ' - ~ */ }
static bool isHexChar(const char ch) {
    return (ch >= 48 && ch <= 57)   /* 0-9 */
        || (ch >= 65 && ch <= 70)   /* A-F */
        || (ch >= 97 && ch <= 102); /* a-f */
}

size_t CharBuff_WriteJsonParseErr(CharBuff* dst, const JsonParseErr err) {
    assert(dst != NULL);
    switch (err) {
        case JSON_PARSE_ERROR_OK:
            return CharBuff_WriteStr(dst, STR("OK"));
        case JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED:
            return CharBuff_WriteStr(dst, STR("NODE_POOL_EXHAUSTED"));
        case JSON_PARSE_ERROR_INVALID:
            return CharBuff_WriteStr(dst, STR("INVALID"));
        case JSON_PARSE_ERROR_PARTIAL:
            return CharBuff_WriteStr(dst, STR("PARTIAL"));
        default:
            return 0;
    }
}

size_t CharBuff_WriteJsonParseResult(CharBuff* dst, const JsonParseResult* r) {
    assert(dst != NULL);
    assert(r != NULL);

    size_t written = 0;
    written += CharBuff_WriteChar(dst, '{');
    written += CharBuff_WriteJsonParseErr(dst, r->err);
    written += CharBuff_WriteChar(dst, ',');
    written += CharBuff_WriteF(dst, "%zd", r->offset);
    written += CharBuff_WriteChar(dst, '}');
    return written;
}

constexpr char TIME_FORMAT_ISO8601[] = "%Y-%m-%dT%H:%M:%S.000Z";

bool Time_ParseISO8601(time_t* dst, Str src) {
    constexpr size_t buffLen = 64;
    assert(src.len < buffLen);
    char      buff[buffLen] = {};
    struct tm tm            = {};

    memcpy(buff, src.arr, src.len * sizeof(char));
    buff[src.len] = '\0';

    if (strptime(buff, TIME_FORMAT_ISO8601, &tm) == NULL) {
        return false;
    }

    *dst = mktime(&tm);
    return true;
}

JsonNode* JsonNodes_At(const JsonNodes nodes, const size_t index) {
    assert(index < nodes.len);
    return &nodes.arr[index];
}

/**
 * Allocates a fresh unused node from the node pool.
 */
JsonNode* JsonNodes_Push(JsonNodes* dst) {
    if (dst->len + 1 > dst->cap) {
        return NULL;
    }

    JsonNode* n = &dst->arr[dst->len];
    dst->len += 1;
    return n;
}

/**
 * Fills the next available node with JSON primitive.
 */
static JsonParseResult JsonNodes_ParsePrimitive(JsonNodes* dst, const Str src, size_t offset) {
    assert(dst != NULL);

    const size_t start = offset;
    for (; offset < src.len; offset++) {
        const char c = Str_At(src, offset);
        if (c == '\0') {
            break;
        }

        switch (c) {
            case ':':
            case '\t':
            case '\r':
            case '\n':
            case ' ':
            case ',':
            case ']':
            case '}':
                JsonNode* const n = JsonNodes_Push(dst);
                if (n == NULL) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED,
                        .offset = offset,
                    };
                }

                *n = (JsonNode){
                    .type     = JSON_NODE_TYPE_PRIMITIVE,
                    .offset   = start,
                    .len      = offset - start,
                    .finished = true,
                };

                offset--;
                return (JsonParseResult){
                    .err    = JSON_PARSE_ERROR_OK,
                    .offset = offset,
                };
            default:
                /* to quiet a warning from gcc*/
                break;
        }
        if (!isVisibleChar(c)) {
            return (JsonParseResult){
                .err    = JSON_PARSE_ERROR_INVALID,
                .offset = offset,
            };
        }
    }

    /* In strict mode primitive must be followed by a comma/object/array */
    return (JsonParseResult){
        .err    = JSON_PARSE_ERROR_PARTIAL,
        .offset = offset,
    };
}

/**
 * Fills the next node with JSON string.
 */
static JsonParseResult JsonNodes_ParseString(JsonNodes* dst, const Str src, size_t offset) {
    assert(dst != NULL);

    const size_t start = offset;
    /* Skip starting quote */
    offset++;

    for (; offset < src.len; offset++) {
        const char c = Str_At(src, offset);
        if (c == '\0') {
            break;
        }

        /* Quote: end of string */
        if (c == '\"') {
            JsonNode* const n = JsonNodes_Push(dst);
            if (n == NULL) {
                return (JsonParseResult){
                    .err    = JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED,
                    .offset = offset,
                };
            }

            *n = (JsonNode){
                .type     = JSON_NODE_TYPE_STRING,
                .offset   = start + 1,
                .len      = offset - (start + 1),
                .finished = true,
            };

            return (JsonParseResult){
                .err    = JSON_PARSE_ERROR_OK,
                .offset = offset,
            };
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && offset + 1 < src.len) {
            offset++;
            const char secondCh = Str_At(src, offset);
            switch (secondCh) {
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

                    for (size_t i = 0; i < 4 && offset < src.len; i++) {
                        const char thirdCh = Str_At(src, offset);
                        if (thirdCh == '\0') {
                            break;
                        }

                        /* If it isn't a hex character, we have an error */
                        if (!isHexChar(thirdCh)) {
                            return (JsonParseResult){
                                .err    = JSON_PARSE_ERROR_INVALID,
                                .offset = offset,
                            };
                        }
                        offset++;
                    }
                    offset--;
                    break;
                /* Unexpected symbol */
                default:
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_INVALID,
                        .offset = offset,
                    };
            }
        }
    }

    return (JsonParseResult){
        .err    = JSON_PARSE_ERROR_PARTIAL,
        .offset = offset,
    };
}

/**
 * Parse JSON string and fill nodes.
 */
JsonParseResult JsonNodes_Parse(JsonNodes* dst, const Str src) {
    assert(dst != NULL);

    int    parentNodeIndex = -1;
    size_t offset          = 0;
    for (; offset < src.len; offset++) {
        const char c = Str_At(src, offset);
        if (c == '\0') {
            break;
        }
        switch (c) {
            case '{':
            case '[': {
                JsonNode* const n = JsonNodes_Push(dst);
                if (n == NULL) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED,
                        .offset = offset,
                    };
                }

                if (parentNodeIndex != -1) {
                    JsonNode* parentT = JsonNodes_At(*dst, parentNodeIndex);

                    /* In strict mode an object or array can't become a key */
                    if (parentT->type == JSON_NODE_TYPE_OBJECT) {
                        return (JsonParseResult){

                            .err    = JSON_PARSE_ERROR_INVALID,
                            .offset = offset,
                        };
                    }

                    parentT->childrenCount++;
                }

                *n = (JsonNode){
                    .type     = (c == '{' ? JSON_NODE_TYPE_OBJECT : JSON_NODE_TYPE_ARRAY),
                    .offset   = offset,
                    .len      = 0,
                    .finished = false,
                };

                parentNodeIndex = (int)dst->len - 1;
                break;
            }
            case '}':
            case ']': {
                if (parentNodeIndex == -1) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_INVALID,
                        .offset = offset,
                    };
                }

                const JsonNodeType type = c == '}' ? JSON_NODE_TYPE_OBJECT : JSON_NODE_TYPE_ARRAY;
                int                i    = (int)dst->len - 1;
                for (; i >= 0; i--) {
                    JsonNode* const n = JsonNodes_At(*dst, i);
                    if (n->finished) {
                        continue;
                    }
                    if (n->type != type) {
                        return (JsonParseResult){
                            .err    = JSON_PARSE_ERROR_INVALID,
                            .offset = offset,
                        };
                    }
                    parentNodeIndex = -1;
                    n->len          = offset - n->offset + 1;
                    n->finished     = true;
                    break;
                }
                /* If unmatched closing bracket, then error*/
                if (i == -1) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_INVALID,
                        .offset = offset,
                    };
                }
                for (; i >= 0; i--) {
                    JsonNode* const n = JsonNodes_At(*dst, i);
                    if (!n->finished) {
                        parentNodeIndex = i;
                        break;
                    }
                }

                if (parentNodeIndex == -1) {
                    offset++;
                    goto finished;
                }
                break;
            }
            case '\"': {
                if (parentNodeIndex == -1) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_INVALID,
                        .offset = offset,
                    };
                }

                const JsonParseResult r = JsonNodes_ParseString(dst, src, offset);
                if (r.err != JSON_PARSE_ERROR_OK) {
                    return r;
                }

                offset                  = r.offset;
                JsonNode* const parentT = JsonNodes_At(*dst, parentNodeIndex);
                parentT->childrenCount++;
                break;
            }
            case '\t':
            case '\r':
            case '\n':
            case ' ':
                break;
            case ':': {
                if (parentNodeIndex == -1) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_INVALID,
                        .offset = offset,
                    };
                }

                parentNodeIndex = (int)dst->len - 1;
                break;
            }
            case ',': {
                if (parentNodeIndex != -1) {
                    JsonNode* const parenT = JsonNodes_At(*dst, parentNodeIndex);
                    if (parenT->type == JSON_NODE_TYPE_ARRAY || parenT->type == JSON_NODE_TYPE_OBJECT) {
                        break;
                    }
                    for (int i = (int)dst->len - 1; i >= 0; i--) {
                        JsonNode* const n = JsonNodes_At(*dst, i);
                        if (n->type == JSON_NODE_TYPE_ARRAY || n->type == JSON_NODE_TYPE_OBJECT) {
                            if (!n->finished) {
                                parentNodeIndex = i;
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
                if (parentNodeIndex == -1) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_INVALID,
                        .offset = offset,
                    };
                }

                JsonNode* const n = JsonNodes_At(*dst, parentNodeIndex);
                if (n->type == JSON_NODE_TYPE_OBJECT || (n->type == JSON_NODE_TYPE_STRING && n->childrenCount != 0)) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_INVALID,
                        .offset = offset,
                    };
                }

                const JsonParseResult r = JsonNodes_ParsePrimitive(dst, src, offset);
                if (r.err != JSON_PARSE_ERROR_OK) {
                    return r;
                }

                offset                 = r.offset;
                JsonNode* const parenT = JsonNodes_At(*dst, parentNodeIndex);
                parenT->childrenCount++;
                break;
            }

            /* Unexpected char in strict mode */
            default:
                return (JsonParseResult){
                    .err    = JSON_PARSE_ERROR_INVALID,
                    .offset = offset,
                };
        }
    }

finished:
    for (int i = (int)dst->len - 1; i >= 0; i--) {
        /* Unmatched opened object or array */
        JsonNode* const n = JsonNodes_At(*dst, i);
        if (!n->finished) {
            return (JsonParseResult){
                .err    = JSON_PARSE_ERROR_PARTIAL,
                .offset = offset,
            };
        }
    }

    return (JsonParseResult){
        .err    = JSON_PARSE_ERROR_OK,
        .offset = offset,
    };
}

JsonStackEntry* JsonStack_Push(JsonStack* s) {
    assert(s != NULL);
    assert(s->cap - s->len > 0);

    return &s->arr[s->len++];
}

JsonStackEntry* JsonStack_Top(JsonStack* s) {
    assert(s != NULL);
    assert(s->len > 0);
    return &s->arr[s->len - 1];
}

JsonStackEntry JsonStack_Pull(JsonStack* s) {
    assert(s != NULL);
    assert(s->len > 0);
    return s->arr[--s->len];
}

size_t CharBuff_WriteJsonStart(CharBuff* dst, JsonStack* s, const char bracket) {
    assert(dst != NULL);
    assert(s != NULL);

    JsonStackEntryType t = JSON_STACK_ENTRY_TYPE_UNSPECIFIED;
    switch (bracket) {
        case '{':
            t = JSON_STACK_ENTRY_TYPE_OBJECT;
            break;
        case '[':
            t = JSON_STACK_ENTRY_TYPE_ARRAY;
            break;
        default:
            assert(false);
    }

    size_t written = 0;
    if (s->len > 0) {
        JsonStackEntry* const parent = JsonStack_Top(s);
        assert(parent != NULL);

        switch (parent->type) {
            case JSON_STACK_ENTRY_TYPE_FIELD:
                break;
            case JSON_STACK_ENTRY_TYPE_ARRAY:
                if (parent->needsComma) {
                    written += CharBuff_WriteChar(dst, ',');
                    break;
                }
                parent->needsComma = true;
                break;
            default:
                assert(false);
        }
    }

    written += CharBuff_WriteChar(dst, bracket);
    JsonStackEntry* const next = JsonStack_Push(s);
    assert(next != NULL);

    *next = (JsonStackEntry){.type = t};
    return written;
}

size_t CharBuff_WriteJsonEnd(CharBuff* dst, JsonStack* s) {
    assert(dst != NULL);
    assert(s != NULL);
    assert(s->len > 0);

    const JsonStackEntry pulled = JsonStack_Pull(s);

    size_t written = 0;
    switch (pulled.type) {
        case JSON_STACK_ENTRY_TYPE_OBJECT:
            written += CharBuff_WriteChar(dst, '}');
            break;
        case JSON_STACK_ENTRY_TYPE_ARRAY:
            written += CharBuff_WriteChar(dst, ']');
            break;
        default:
            assert(false);
    }

    if (s->len > 0 && JsonStack_Top(s)->type == JSON_STACK_ENTRY_TYPE_FIELD) {
        JsonStack_Pull(s);
    }

    return written;
}

size_t CharBuff_WriteJsonKey(CharBuff* dst, JsonStack* s, Str key) {
    assert(dst != NULL);
    assert(s != NULL);
    assert(Str_IsValid(key));

    JsonStackEntry* const parent = JsonStack_Top(s);
    assert(parent != NULL);
    assert(parent->type == JSON_STACK_ENTRY_TYPE_OBJECT);

    size_t written = 0;

    if (parent->needsComma) {
        written += CharBuff_WriteChar(dst, ',');
    }
    parent->needsComma = true;

    JsonStackEntry* const next = JsonStack_Push(s);
    assert(next != NULL);

    *next = (JsonStackEntry){.type = JSON_STACK_ENTRY_TYPE_FIELD};

    written += CharBuff_WriteChar(dst, '\"');
    written += CharBuff_WriteStr(dst, key);
    written += CharBuff_WriteChar(dst, '\"');
    written += CharBuff_WriteChar(dst, ':');

    return written;
}

size_t CharBuff_WritePrimitivePreamble(CharBuff* dst, JsonStack* s) {
    assert(dst != NULL);
    assert(s != NULL);
    assert(s->len > 0);

    size_t                written = 0;
    JsonStackEntry* const parent  = JsonStack_Top(s);
    switch (parent->type) {
        case JSON_STACK_ENTRY_TYPE_FIELD:
            JsonStack_Pull(s);
            parent->needsComma = true;
            break;
        case JSON_STACK_ENTRY_TYPE_ARRAY:
            if (parent->needsComma) {
                written += CharBuff_WriteChar(dst, ',');
                break;
            }
            parent->needsComma = true;
            break;
        default:
            assert(false);
    }

    return written;
}

size_t CharBuff_WriteJsonValue(CharBuff* dst, JsonStack* s, const bool isString, const Str value) {
    assert(dst != NULL);
    assert(s != NULL);

    size_t written = 0;
    written += CharBuff_WritePrimitivePreamble(dst, s);
    if (isString) {
        written += CharBuff_WriteChar(dst, '\"');
        for (size_t i = 0; i < value.len; ++i) {
            const char ch = Str_At(value, i);
            switch (ch) {
                case '\"':
                    written += CharBuff_WriteStr(dst, STR("\\\""));
                    break;
                case '\\':
                    written += CharBuff_WriteStr(dst, STR("\\\\"));
                    break;
                case '\b':
                    written += CharBuff_WriteStr(dst, STR("\\b"));
                    break;
                case '\f':
                    written += CharBuff_WriteStr(dst, STR("\\f"));
                    break;
                case '\n':
                    written += CharBuff_WriteStr(dst, STR("\\n"));
                    break;
                case '\r':
                    written += CharBuff_WriteStr(dst, STR("\\r"));
                    break;
                case '\t':
                    written += CharBuff_WriteStr(dst, STR("\\t"));
                    break;
                default:
                    written += CharBuff_WriteChar(dst, ch);
            }
        }
        written += CharBuff_WriteChar(dst, '\"');
        return written;
    }

    assert(value.len > 0);
    const char firstCh = Str_At(value, 0);
    switch (firstCh) {
        case 'n':
            assert(Str_Equals(value, STR("null")));
            return CharBuff_WriteStr(dst, STR("null"));
        case 't':
            assert(Str_Equals(value, STR("true")));
            return CharBuff_WriteStr(dst, STR("true"));
        case 'f':
            assert(Str_Equals(value, STR("false")));
            return CharBuff_WriteStr(dst, STR("false"));
        default:
            if (!(firstCh == '-' || (firstCh >= '0' && firstCh <= '9'))) {
                assert(false);
            }
            written += CharBuff_WriteChar(dst, firstCh);
            bool wasPoint = false;
            for (size_t i = 1; i < value.len; ++i) {
                const char ch = Str_At(value, i);
                if (ch == '.') {
                    if (wasPoint) {
                        assert(false);
                    }
                    wasPoint = true;
                    written += CharBuff_WriteChar(dst, '.');
                } else if (ch >= '0' && ch <= '9') {
                    written += CharBuff_WriteChar(dst, ch);
                } else {
                    assert(false);
                }
            }
            return written;
    }
}

size_t CharBuff_WriteJsonStr(CharBuff* dst, JsonStack* s, const Str value) {
    return CharBuff_WriteJsonValue(dst, s, true, value);
}

size_t CharBuff_WriteJsonBool(CharBuff* dst, JsonStack* s, const bool value) {
    assert(dst != NULL);
    assert(s != NULL);
    return CharBuff_WriteJsonValue(dst, s, false, value ? STR("true") : STR("false"));
}

size_t CharBuff_WriteJsonNull(CharBuff* dst, JsonStack* s) {
    assert(dst != NULL);
    assert(s != NULL);
    return CharBuff_WriteJsonValue(dst, s, false, STR("null"));
}
size_t CharBuff_WriteJsonTime(CharBuff* dst, JsonStack* s, const time_t t) {
    assert(dst != NULL);
    assert(s != NULL);

    constexpr size_t BUFF_SIZE = 64;
    CharBuff         v         = CharBuff_OnStack(0, BUFF_SIZE);
    const size_t     written   = CharBuff_WriteTimeISO8601(&v, t);
    if (written == 0) {
        return 0;
    }

    return CharBuff_WriteJsonStr(dst, s, CharBuff_ToStr(v));
}

size_t CharBuff_WriteJsonNumeric(CharBuff* dst, JsonStack* s, const Str value) {
    assert(dst != NULL);
    assert(s != NULL);

    return CharBuff_WriteJsonValue(dst, s, false, value);
}
size_t CharBuff_WriteTimeISO8601(CharBuff* dst, const time_t t) {
    const struct tm* const tm        = gmtime(&t);
    const size_t           remaining = dst->cap - dst->len;
    const size_t           len       = strftime(dst->arr, remaining, TIME_FORMAT_ISO8601, tm);
    dst->len += len;
    return len;
}

void JsonSrc_Reset(JsonSrc* s) { s->index = 0; }

static const JsonNode* JsonSource_Node(const JsonSrc* s) { return JsonNodes_At(s->nodes, s->index); }

bool JsonSrc_Next(JsonSrc* s) {
    if (s->index >= s->nodes.len) {
        return false;
    }

    s->index++;
    return true;
}

bool JsonSrc_Skip(JsonSrc* s) {
    const JsonNode* const n = JsonSource_Node(s);
    if (n->childrenCount == 0) {
        return false;
    }

    for (size_t i = 0; i < n->childrenCount; i++) {
        JsonSrc_Next(s);
        JsonSrc_Skip(s);
    }

    return true;
}

JsonType JsonSrc_Type(const JsonSrc* s) {
    const JsonNode* const n = JsonSource_Node(s);
    switch (n->type) {
        case JSON_NODE_TYPE_OBJECT:
            return JSON_TYPE_OBJECT;
        case JSON_NODE_TYPE_ARRAY:
            return JSON_TYPE_ARRAY;
        case JSON_NODE_TYPE_STRING:
            switch (n->childrenCount) {
                case 0:
                    return JSON_TYPE_STRING;
                case 1:
                    return JSON_TYPE_KEY;
                default:
                    assert(false);
            }

        case JSON_NODE_TYPE_PRIMITIVE:
            assert(n->childrenCount == 0);
            const Str v = JsonSrc_Value(s);
            assert(v.len > 0);
            const char firstCh = Str_At(v, 0);
            switch (firstCh) {
                case 'n':
                    return JSON_TYPE_NULL;
                case 't':
                case 'f':
                    return JSON_TYPE_BOOL;
                default:
                    return JSON_TYPE_NUMERIC;
            }
        default:
            assert(false);
    }
}

Str JsonSrc_Value(const JsonSrc* s) {
    JsonNode* const n = JsonNodes_At(s->nodes, s->index);
    return Str_View(s->str, n->offset, n->offset + n->len);
}

bool JsonSrc_BoolValue(const JsonSrc* s) {
    assert(JsonSrc_Type(s) == JSON_TYPE_BOOL);
    const Str v = JsonSrc_Value(s);
    assert(v.len > 0);
    const char firstCh = Str_At(v, 0);

    switch (firstCh) {
        case 't':
            return true;
        case 'f':
            return false;
        default:
            assert(false);
    }
}

size_t JsonSrc_ChildrenCount(const JsonSrc* s) {
    const JsonNode* const n = JsonSource_Node(s);
    return n->childrenCount;
}
