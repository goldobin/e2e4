#include "json.h"

#include <assert.h>

static bool isVisibleChar(const char ch) { return ch >= 32 && ch <= 126; /* ' ' - ~ */ }
static bool isHexChar(const char ch) {
    return (ch >= 48 && ch <= 57)   /* 0-9 */
        || (ch >= 65 && ch <= 70)   /* A-F */
        || (ch >= 97 && ch <= 102); /* a-f */
}

size_t CharSlice_WriteJsonParseErr(CharSlice *dst, const JsonParseErr err) {
    assert(dst != nullptr);
    switch (err) {
        case JSON_PARSE_ERROR_OK:
            return CharSlice_WriteStr(dst, CHAR_SLICE("OK"));
        case JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED:
            return CharSlice_WriteStr(dst, CHAR_SLICE("NODE_POOL_EXHAUSTED"));
        case JSON_PARSE_ERROR_INVALID:
            return CharSlice_WriteStr(dst, CHAR_SLICE("INVALID"));
        case JSON_PARSE_ERROR_PARTIAL:
            return CharSlice_WriteStr(dst, CHAR_SLICE("PARTIAL"));
        default:
            return 0;
    }
}

size_t CharSlice_WriteJsonParseResult(CharSlice *dst, const JsonParseResult *r) {
    assert(dst != nullptr);
    assert(r != nullptr);

    size_t written = 0;
    written += CharSlice_WriteChar(dst, '{');
    written += CharSlice_WriteJsonParseErr(dst, r->err);
    written += CharSlice_WriteChar(dst, ',');
    written += CharSlice_WriteF(dst, "%zd", r->offset);
    written += CharSlice_WriteChar(dst, '}');
    return written;
}

JsonNode *JsonNodes_At(const JsonNodes nodes, const size_t index) {
    assert(index < nodes.len);
    return &nodes.arr[index];
}

/**
 * Allocates a fresh unused node from the node pool.
 */
JsonNode *JsonNodes_Push(JsonNodes *dst) {
    if (dst->len + 1 > dst->cap) {
        return nullptr;
    }

    JsonNode *n = &dst->arr[dst->len];
    dst->len += 1;
    return n;
}

// size_t JsonNodes_Skip(const JsonNodes nodes, size_t index) {
//     assert(nodes != nullptr);
//     assert(index < nodes->len);
//
//     const auto n = JsonNodes_At(nodes, index);
//     for (size_t i = 0; i < n->childrenCount; i++) {
//         index = JsonNodes_Skip(nodes, ++index);
//     }
//
//     return index;
// }

/**
 * Fills the next available node with JSON primitive.
 */
static JsonParseResult JsonNodes_ParsePrimitive(JsonNodes *dst, const Str src, size_t offset) {
    assert(dst != nullptr);

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
                const auto n = JsonNodes_Push(dst);
                if (n == nullptr) {
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
static JsonParseResult JsonNodes_ParseString(JsonNodes *dst, const Str src, size_t offset) {
    assert(dst != nullptr);

    const auto start = offset;
    /* Skip starting quote */
    offset++;

    for (; offset < src.len; offset++) {
        const char c = Str_At(src, offset);
        if (c == '\0') {
            break;
        }

        /* Quote: end of string */
        if (c == '\"') {
            const auto n = JsonNodes_Push(dst);
            if (n == nullptr) {
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
JsonParseResult JsonNodes_Parse(JsonNodes *dst, const Str src) {
    assert(dst != nullptr);

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
                const auto n = JsonNodes_Push(dst);
                if (n == nullptr) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED,
                        .offset = offset,
                    };
                }

                if (parentNodeIndex != -1) {
                    JsonNode *parentT = JsonNodes_At(*dst, parentNodeIndex);

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

                const JsonNodeType type = (c == '}' ? JSON_NODE_TYPE_OBJECT : JSON_NODE_TYPE_ARRAY);
                int                i    = (int)dst->len - 1;
                for (; i >= 0; i--) {
                    const auto n = JsonNodes_At(*dst, i);
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
                    const auto n = JsonNodes_At(*dst, i);
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

                const auto r = JsonNodes_ParseString(dst, src, offset);
                if (r.err != JSON_PARSE_ERROR_OK) {
                    return r;
                }

                offset             = r.offset;
                const auto parentT = JsonNodes_At(*dst, parentNodeIndex);
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
                    const auto parenT = JsonNodes_At(*dst, parentNodeIndex);
                    if (parenT->type == JSON_NODE_TYPE_ARRAY || parenT->type == JSON_NODE_TYPE_OBJECT) {
                        break;
                    }
                    for (int i = (int)dst->len - 1; i >= 0; i--) {
                        const auto n = JsonNodes_At(*dst, i);
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

                const auto n = JsonNodes_At(*dst, parentNodeIndex);
                if (n->type == JSON_NODE_TYPE_OBJECT || (n->type == JSON_NODE_TYPE_STRING && n->childrenCount != 0)) {
                    return (JsonParseResult){
                        .err    = JSON_PARSE_ERROR_INVALID,
                        .offset = offset,
                    };
                }

                const auto r = JsonNodes_ParsePrimitive(dst, src, offset);
                if (r.err != JSON_PARSE_ERROR_OK) {
                    return r;
                }

                offset            = r.offset;
                const auto parenT = JsonNodes_At(*dst, parentNodeIndex);
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
        const auto n = JsonNodes_At(*dst, i);
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

JsonStackEntry *JsonStack_Push(JsonStack *s) {
    assert(s != nullptr);
    assert(s->cap - s->len > 0);

    return &s->arr[s->len++];
}

JsonStackEntry *JsonStack_Top(const JsonStack *s) {
    assert(s != nullptr);
    assert(s->len > 0);
    return &s->arr[s->len - 1];
}

JsonStackEntry JsonStack_Pull(JsonStack *s) {
    assert(s != nullptr);
    assert(s->len > 0);
    return s->arr[--s->len];
}

size_t CharSlice_WriteJsonStart(CharSlice *dst, JsonStack *s, const char bracket) {
    assert(dst != nullptr);
    assert(s != nullptr);

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

    if (s->len > 0) {
        const auto n = JsonStack_Top(s);
        assert(n != nullptr);
        assert(n->type == JSON_STACK_ENTRY_TYPE_FIELD);
    }

    size_t written = 0;
    written += CharSlice_WriteChar(dst, bracket);
    const auto next = JsonStack_Push(s);
    assert(next != nullptr);

    *next = (JsonStackEntry){.type = t};
    return written;
}

size_t CharSlice_WriteJsonEnd(CharSlice *dst, JsonStack *s) {
    assert(dst != nullptr);
    assert(s != nullptr);
    assert(s->len > 0);

    const auto pulled = JsonStack_Pull(s);

    size_t written = 0;
    switch (pulled.type) {
        case JSON_STACK_ENTRY_TYPE_OBJECT:
            written += CharSlice_WriteChar(dst, '}');
            break;
        case JSON_STACK_ENTRY_TYPE_ARRAY:
            written += CharSlice_WriteChar(dst, ']');
            break;
        default:
            assert(false);
    }

    if (s->len > 0 && JsonStack_Top(s)->type == JSON_STACK_ENTRY_TYPE_FIELD) {
        JsonStack_Pull(s);
    }

    return written;
}

size_t CharSlice_WriteJsonKey(CharSlice *dst, JsonStack *s, Str key) {
    assert(dst != nullptr);
    assert(s != nullptr);
    assert(Str_IsValid(key));

    const auto parent = JsonStack_Top(s);
    assert(parent != nullptr);
    assert(parent->type == JSON_STACK_ENTRY_TYPE_OBJECT);

    size_t written = 0;

    if (parent->needsComma) {
        written += CharSlice_WriteChar(dst, ',');
    }
    parent->needsComma = true;

    const auto next = JsonStack_Push(s);
    assert(next != nullptr);

    *next = (JsonStackEntry){.type = JSON_STACK_ENTRY_TYPE_FIELD};

    written += CharSlice_WriteChar(dst, '\"');
    written += CharSlice_WriteStr(dst, key);
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
        case JSON_STACK_ENTRY_TYPE_FIELD:
            JsonStack_Pull(s);
            parent->needsComma = true;
            break;
        case JSON_STACK_ENTRY_TYPE_ARRAY:
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

size_t CharSlice_WriteJsonValue(CharSlice *dst, JsonStack *s, const bool isString, const Str value) {
    assert(dst != nullptr);
    assert(s != nullptr);

    size_t written = 0;
    written += CharSlice_WritePrimitivePreamble(dst, s);
    if (isString) {
        written += CharSlice_WriteChar(dst, '\"');
        for (size_t i = 0; i < value.len; ++i) {
            const auto ch = Str_At(value, i);
            switch (ch) {
                case '\"':
                    written += CharSlice_WriteStr(dst, CHAR_SLICE("\\\""));
                    break;
                case '\\':
                    written += CharSlice_WriteStr(dst, CHAR_SLICE("\\\\"));
                case '\b':
                    written += CharSlice_WriteStr(dst, CHAR_SLICE("\\\b"));
                case '\f':
                    written += CharSlice_WriteStr(dst, CHAR_SLICE("\\\f"));
                case '\n':
                    written += CharSlice_WriteStr(dst, CHAR_SLICE("\\\n"));
                case '\r':
                    written += CharSlice_WriteStr(dst, CHAR_SLICE("\\\r"));
                case '\t':
                    written += CharSlice_WriteStr(dst, CHAR_SLICE("\\\t"));
                default:
                    written += CharSlice_WriteChar(dst, ch);
            }
        }
        written += CharSlice_WriteChar(dst, '\"');
        return written;
    }

    assert(value.len > 0);
    const auto firstCh = Str_At(value, 0);
    switch (firstCh) {
        case 'n':
            assert(Str_Equals(value, CHAR_SLICE("null")));
            return CharSlice_WriteStr(dst, CHAR_SLICE("null"));
        case 't':
            assert(Str_Equals(value, CHAR_SLICE("true")));
            return CharSlice_WriteStr(dst, CHAR_SLICE("true"));
        case 'f':
            assert(Str_Equals(value, CHAR_SLICE("false")));
            return CharSlice_WriteStr(dst, CHAR_SLICE("false"));
        default:
            if (!(firstCh == '-' || (firstCh >= '0' && firstCh <= '9'))) {
                assert(false);
            }
            written += CharSlice_WriteChar(dst, firstCh);
            bool wasPoint = false;
            for (size_t i = 1; i < value.len; ++i) {
                const auto ch = Str_At(value, i);
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

size_t CharSlice_WriteJsonStr(CharSlice *dst, JsonStack *s, const Str value) {
    return CharSlice_WriteJsonValue(dst, s, true, value);
}

size_t CharSlice_WriteJsonBool(CharSlice *dst, JsonStack *s, const bool value) {
    assert(dst != nullptr);
    assert(s != nullptr);
    return CharSlice_WriteJsonValue(dst, s, false, value ? CHAR_SLICE("true") : CHAR_SLICE("false"));
}

size_t CharSlice_WriteJsonNull(CharSlice *dst, JsonStack *s) {
    assert(dst != nullptr);
    assert(s != nullptr);
    return CharSlice_WriteJsonValue(dst, s, false, CHAR_SLICE("null"));
}

size_t CharSlice_WriteJsonNumeric(CharSlice *dst, JsonStack *s, const Str value) {
    assert(dst != nullptr);
    assert(s != nullptr);

    return CharSlice_WriteJsonValue(dst, s, false, value);
}

void JsonSource_Reset(JsonSource *s) { s->index = 0; }

static const JsonNode *JsonSource_Node(const JsonSource *s) { return JsonNodes_At(s->nodes, s->index); }

bool JsonSource_Next(JsonSource *s) {
    if (s->index >= s->nodes.len) {
        return false;
    }

    s->index++;
    return true;
}

bool JsonSource_Skip(JsonSource *s) {
    const auto n = JsonSource_Node(s);
    if (n->childrenCount == 0) {
        return false;
    }

    for (size_t i = 0; i < n->childrenCount; i++) {
        JsonSource_Next(s);
        JsonSource_Skip(s);
    }

    return true;
}

JsonType JsonSource_Type(const JsonSource *s) {
    const auto n = JsonSource_Node(s);
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
            const auto v = JsonSource_Value(s);
            assert(v.len > 0);
            const auto firstCh = Str_At(v, 0);
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

Str JsonSource_Value(const JsonSource *s) {
    const auto n = JsonNodes_At(s->nodes, s->index);
    return Str_View(s->str, n->offset, n->offset + n->len);
}

bool JsonSource_BoolValue(const JsonSource *s) {
    assert(JsonSource_Type(s) == JSON_TYPE_BOOL);
    const auto v = JsonSource_Value(s);
    assert(v.len > 0);
    const auto firstCh = Str_At(v, 0);

    switch (firstCh) {
        case 't':
            return true;
        case 'f':
            return false;
        default:
            assert(false);
    }
}

size_t JsonSource_ChildrenCount(const JsonSource *s) {
    const auto n = JsonSource_Node(s);
    return n->childrenCount;
}
