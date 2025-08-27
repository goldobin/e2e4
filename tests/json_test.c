#include "json.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "arena.h"

uint8_t buffer[1024 * 1024] = {};
Arena   mem                 = {};

void setUp() {}
void tearDown() {}

void vtokeq(const JsonNodes *ts, const CharSlice src, va_list ap) {
    assert(ts != nullptr);

    if (ts->len < 1) {
        return;
    }

    for (size_t i = 0; i < ts->len; i++) {
        size_t offset        = 0;
        size_t len           = 0;
        size_t childrenCount = 0;
        char  *value         = nullptr;

        const JsonNodeType type = va_arg(ap, JsonNodeType);
        if (type == JSON_NODE_TYPE_STRING) {
            value         = va_arg(ap, char *);
            childrenCount = va_arg(ap, int);
        } else if (type == JSON_NODE_TYPE_PRIMITIVE) {
            value = va_arg(ap, char *);
        } else {
            offset        = va_arg(ap, int);
            const int end = va_arg(ap, int);
            len           = end - offset;
            childrenCount = va_arg(ap, int);
        }

        const auto n = JsonNodes_At(ts, i);
        TEST_ASSERT_EQUAL(type, n->type);

        if (len != 0) {
            TEST_ASSERT_EQUAL(offset, n->offset);
            TEST_ASSERT_EQUAL(len, n->len);
        }

        TEST_ASSERT_EQUAL(childrenCount, n->childrenCount);
        if (value != nullptr) {
            const auto view = CharSlice_View(src, n->offset, n->offset + n->len);
            if (view.len != n->len || strncmp(value, view.arr, view.len) != 0) {
                printf("node %lu value is %.*s, not %s\n", i, (int)view.len, view.arr, value);
                TEST_FAIL();
            }
        }
    }
}

static void tokeq(const JsonNodes *ts, CharSlice src, ...) {
    assert(ts != nullptr);

    va_list args;
    va_start(args, numtok);
    vtokeq(ts, src, args);
    va_end(args);
}

void run_parse_case(
    const CharSlice src, const size_t poolSize, const JsonParseErr wantErr, const size_t wantNodeCount, ...
) {
    JsonNodes ts = {
        .arr = Arena_Alloc(&mem, poolSize * sizeof(JsonNode)),
        .cap = poolSize,
    };
    const auto r = JsonNodes_Parse(&ts, src);

    TEST_ASSERT_EQUAL(wantErr, r.err);

    if (wantErr != JSON_PARSE_ERROR_OK) {
        return;
    }

    TEST_ASSERT_EQUAL(ts.len, wantNodeCount);
    if (wantNodeCount > 0) {
        va_list args;
        va_start(args, wantNodeCount);
        vtokeq(&ts, src, args);
        va_end(args);
    }
}

void test_empty() {
    run_parse_case(CHAR_SLICE("{}"), 1, JSON_PARSE_ERROR_OK, 1, JSON_NODE_TYPE_OBJECT, 0, 2, 0);
    run_parse_case(CHAR_SLICE("[]"), 1, JSON_PARSE_ERROR_OK, 1, JSON_NODE_TYPE_ARRAY, 0, 2, 0);
    run_parse_case(
        CHAR_SLICE("[{},{}]"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_ARRAY, 0, 7, 2, JSON_NODE_TYPE_OBJECT, 1, 3, 0,
        JSON_NODE_TYPE_OBJECT, 4, 6, 0
    );
}

void test_object() {
    run_parse_case(
        CHAR_SLICE("{\"a\":0}"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, 0, 7, 1, JSON_NODE_TYPE_STRING, "a",
        1, JSON_NODE_TYPE_PRIMITIVE, "0"
    );
    run_parse_case(
        CHAR_SLICE("{\"a\":[]}"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, 0, 8, 1, JSON_NODE_TYPE_STRING, "a",
        1, JSON_NODE_TYPE_ARRAY, 5, 7, 0
    );
    run_parse_case(
        CHAR_SLICE("{\"a\":{},\"b\":{}}"), 5, JSON_PARSE_ERROR_OK, 5, JSON_NODE_TYPE_OBJECT, -1, -1, 2,
        JSON_NODE_TYPE_STRING, "a", 1, JSON_NODE_TYPE_OBJECT, -1, -1, 0, JSON_NODE_TYPE_STRING, "b", 1,
        JSON_NODE_TYPE_OBJECT, -1, -1, 0
    );
    run_parse_case(
        CHAR_SLICE("{\n \"Day\": 26,\n \"Month\": 9,\n \"Year\": 12\n }"), 7, JSON_PARSE_ERROR_OK, 7,
        JSON_NODE_TYPE_OBJECT, -1, -1, 3, JSON_NODE_TYPE_STRING, "Day", 1, JSON_NODE_TYPE_PRIMITIVE, "26",
        JSON_NODE_TYPE_STRING, "Month", 1, JSON_NODE_TYPE_PRIMITIVE, "9", JSON_NODE_TYPE_STRING, "Year", 1,
        JSON_NODE_TYPE_PRIMITIVE, "12"
    );
    run_parse_case(
        CHAR_SLICE("{\"a\": 0, \"b\": \"c\"}"), 5, JSON_PARSE_ERROR_OK, 5, JSON_NODE_TYPE_OBJECT, -1, -1, 2,
        JSON_NODE_TYPE_STRING, "a", 1, JSON_NODE_TYPE_PRIMITIVE, "0", JSON_NODE_TYPE_STRING, "b", 1,
        JSON_NODE_TYPE_STRING, "c", 0
    );

    run_parse_case(CHAR_SLICE("{\"a\"\n0}"), 3, JSON_PARSE_ERROR_INVALID, 0);
    run_parse_case(CHAR_SLICE("{\"a\", 0}"), 3, JSON_PARSE_ERROR_INVALID, 0);
    run_parse_case(CHAR_SLICE("{\"a\": {2}}"), 3, JSON_PARSE_ERROR_INVALID, 0);
    run_parse_case(CHAR_SLICE("{\"a\": {2: 3}}"), 3, JSON_PARSE_ERROR_INVALID, 0);
    run_parse_case(CHAR_SLICE("{\"a\": {\"a\": 2 3}}"), 5, JSON_PARSE_ERROR_INVALID, 0);
    /* FIXME */
    /*run_parse_case("{\"a\"}", JSON_PARSE_ERROR_INVALID, 2);*/
    /*run_parse_case("{\"a\": 1, \"b\"}", JSON_PARSE_ERROR_INVALID, 4);*/
    /*run_parse_case("{\"a\",\"b\":1}", JSON_PARSE_ERROR_INVALID, 4);*/
    /*run_parse_case("{\"a\":1,}", JSON_PARSE_ERROR_INVALID, 4);*/
    /*run_parse_case("{\"a\":\"b\":\"c\"}", JSON_PARSE_ERROR_INVALID, 4);*/
    /*run_parse_case("{,}", JSON_PARSE_ERROR_INVALID, 4);*/
}

void test_array(void) {
    /* FIXME */
    /*run_parse_case("[10}", JSON_PARSE_ERROR_INVALID, 3);*/
    /*run_parse_case("[1,,3]", JSON_PARSE_ERROR_INVALID, 3)*/
    run_parse_case(
        CHAR_SLICE("[10]"), 2, JSON_PARSE_ERROR_OK, 2, JSON_NODE_TYPE_ARRAY, -1, -1, 1, JSON_NODE_TYPE_PRIMITIVE, "10"
    );
    run_parse_case(CHAR_SLICE("{\"a\": 1]"), 3, JSON_PARSE_ERROR_INVALID, 0);
    /* FIXME */
    /*run_parse_case("[\"a\": 1]", JSON_PARSE_ERROR_INVALID, 3);*/
}

void test_primitive() {
    run_parse_case(
        CHAR_SLICE("{\"boolVar\" : true }"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "boolVar", 1, JSON_NODE_TYPE_PRIMITIVE, "true"
    );
    run_parse_case(
        CHAR_SLICE("{\"boolVar\" : false }"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "boolVar", 1, JSON_NODE_TYPE_PRIMITIVE, "false"
    );
    run_parse_case(
        CHAR_SLICE("{\"nullVar\" : null }"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "nullVar", 1, JSON_NODE_TYPE_PRIMITIVE, "null"
    );
    run_parse_case(
        CHAR_SLICE("{\"intVar\" : 12}"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "intVar", 1, JSON_NODE_TYPE_PRIMITIVE, "12"
    );
    run_parse_case(
        CHAR_SLICE("{\"floatVar\" : 12.345}"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "floatVar", 1, JSON_NODE_TYPE_PRIMITIVE, "12.345"
    );
}

void test_string(void) {
    run_parse_case(
        CHAR_SLICE("{\"strVar\" : \"hello world\"}"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "strVar", 1, JSON_NODE_TYPE_STRING, "hello world", 0
    );
    run_parse_case(
        CHAR_SLICE("{\"strVar\" : \"escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\\"}"), 3, JSON_PARSE_ERROR_OK, 3,
        JSON_NODE_TYPE_OBJECT, -1, -1, 1, JSON_NODE_TYPE_STRING, "strVar", 1, JSON_NODE_TYPE_STRING,
        "escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\", 0
    );
    run_parse_case(
        CHAR_SLICE("{\"strVar\": \"\"}"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "strVar", 1, JSON_NODE_TYPE_STRING, "", 0
    );
    run_parse_case(
        CHAR_SLICE("{\"a\":\"\\uAbcD\"}"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "a", 1, JSON_NODE_TYPE_STRING, "\\uAbcD", 0
    );
    run_parse_case(
        CHAR_SLICE("{\"a\":\"str\\u0000\"}"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "a", 1, JSON_NODE_TYPE_STRING, "str\\u0000", 0
    );
    run_parse_case(
        CHAR_SLICE("{\"a\":\"\\uFFFFstr\"}"), 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "a", 1, JSON_NODE_TYPE_STRING, "\\uFFFFstr", 0
    );
    run_parse_case(
        CHAR_SLICE("{\"a\":[\"\\u0280\"]}"), 4, JSON_PARSE_ERROR_OK, 4, JSON_NODE_TYPE_OBJECT, -1, -1, 1,
        JSON_NODE_TYPE_STRING, "a", 1, JSON_NODE_TYPE_ARRAY, -1, -1, 1, JSON_NODE_TYPE_STRING, "\\u0280", 0
    );
    run_parse_case(CHAR_SLICE("{\"a\":\"str\\uFFGFstr\"}"), 3, JSON_PARSE_ERROR_INVALID, 0);
    run_parse_case(CHAR_SLICE("{\"a\":\"str\\u@FfF\"}"), 3, JSON_PARSE_ERROR_INVALID, 0);
    run_parse_case(CHAR_SLICE("{{\"a\":[\"\\u028\"]}"), 4, JSON_PARSE_ERROR_INVALID, 0);
}

void test_partial_string(void) {
    const auto src = CHAR_SLICE("{\"x\": \"va\\\\ue\", \"y\": \"value y\"}");
    JsonNode   arr[5];
    JsonNodes  dst = {
         .arr = arr,
         .cap = sizeof(arr) / sizeof(JsonNode),
    };

    for (size_t i = 1; i <= src.len; i++) {
        const auto r = JsonNodes_Parse(&dst, CharSlice_View(src, 0, i));
        if (i != src.len) {
            TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_PARTIAL, r.err);
            continue;
        }

        TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, r.err);
        TEST_ASSERT_EQUAL(5, dst.len);
        tokeq(
            &dst, src, JSON_NODE_TYPE_OBJECT, -1, -1, 2, JSON_NODE_TYPE_STRING, "x", 1, JSON_NODE_TYPE_STRING,
            "va\\\\ue", 0, JSON_NODE_TYPE_STRING, "y", 1, JSON_NODE_TYPE_STRING, "value y", 0
        );
    }
}

void test_partial_array(void) {
    const auto src = CHAR_SLICE("[ 1, true, [123, \"hello\"]]");
    JsonNode   arr[10];
    JsonNodes  dst = {
         .arr = arr,
         .cap = sizeof(arr) / sizeof(JsonNode),
    };

    for (size_t i = 1; i <= src.len; i++) {
        const auto r = JsonNodes_Parse(&dst, CharSlice_View(src, 0, i));
        if (i != src.len) {
            TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_PARTIAL, r.err);
            continue;
        }

        TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, r.err);
        TEST_ASSERT_EQUAL(6, dst.len);
        tokeq(
            &dst, src, JSON_NODE_TYPE_ARRAY, -1, -1, 3, JSON_NODE_TYPE_PRIMITIVE, "1", JSON_NODE_TYPE_PRIMITIVE, "true",
            JSON_NODE_TYPE_ARRAY, -1, -1, 2, JSON_NODE_TYPE_PRIMITIVE, "123", JSON_NODE_TYPE_STRING, "hello", 0
        );
    }
}

void test_array_nomem(void) {
    for (int i = 0; i < 6; i++) {
        constexpr size_t arrLen      = 10;
        JsonNode         arr[arrLen] = {};
        JsonNodes        dst         = {.arr = arr, .cap = i};
        const auto       src         = CHAR_SLICE("  [ 1, true, [123, \"hello\"]]");
        const auto       r1          = JsonNodes_Parse(&dst, src);
        TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED, r1.err);
    }
}

void test_unquoted_keys(void) {
    JsonNode   arr[10];
    JsonNodes  dst = {.arr = arr, .cap = sizeof(arr) / sizeof(JsonNode)};
    const auto src = CHAR_SLICE("key1: \"value\"\nkey2 : 123");
    const auto r   = JsonNodes_Parse(&dst, src);
    TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_INVALID, r.err);
    tokeq(
        &dst, src, JSON_NODE_TYPE_PRIMITIVE, "key1", JSON_NODE_TYPE_STRING, "value", 0, JSON_NODE_TYPE_PRIMITIVE,
        "key2", JSON_NODE_TYPE_PRIMITIVE, "123"
    );
}

void test_issue_22(void) {
    JsonNode   arr[128] = {};
    JsonNodes  dst      = {.arr = arr, .cap = sizeof(arr) / sizeof(JsonNode)};
    const auto src      = CHAR_SLICE(
        "{ \"height\":10, \"layers\":[ { \"data\":[6,6], \"height\":10, "
             "\"name\":\"Calque de Tile 1\", \"opacity\":1, \"type\":\"tilelayer\", "
             "\"visible\":true, \"width\":10, \"x\":0, \"y\":0 }], "
             "\"orientation\":\"orthogonal\", \"properties\": { }, \"tileheight\":32, "
             "\"tilesets\":[ { \"firstgid\":1, \"image\":\"..\\/images\\/tiles.png\", "
             "\"imageheight\":64, \"imagewidth\":160, \"margin\":0, "
             "\"name\":\"Tiles\", "
             "\"properties\":{}, \"spacing\":0, \"tileheight\":32, \"tilewidth\":32 "
             "}], "
             "\"tilewidth\":32, \"version\":1, \"width\":10 }"
    );
    const auto r = JsonNodes_Parse(&dst, src);
    TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, r.err);
}

void test_issue_27(void) {
    const auto src = CHAR_SLICE("{ \"name\" : \"Jack\", \"age\" : 27 } { \"name\" : \"Anna\", ");
    run_parse_case(
        src, 8, JSON_PARSE_ERROR_OK, 5, JSON_NODE_TYPE_OBJECT, 0, 31, 2, JSON_NODE_TYPE_STRING, "name", 1,
        JSON_NODE_TYPE_STRING, "Jack", 0, JSON_NODE_TYPE_STRING, "age", 1, JSON_NODE_TYPE_PRIMITIVE, "27"
    );
}

void test_input_length(void) {
    JsonNode  arr[10] = {};
    JsonNodes dst     = {
            .arr = arr,
            .cap = sizeof(arr) / sizeof(JsonNode),
    };
    const auto src = CHAR_SLICE("{\"a\": 0}garbage");
    const auto r   = JsonNodes_Parse(&dst, src);
    TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, r.err);
    TEST_ASSERT_EQUAL(8, r.offset);
    TEST_ASSERT_EQUAL(3, dst.len);
    tokeq(&dst, src, JSON_NODE_TYPE_OBJECT, -1, -1, 1, JSON_NODE_TYPE_STRING, "a", 1, JSON_NODE_TYPE_PRIMITIVE, "0");
}

void test_count(void) {
    typedef struct {
        const char  *name;
        const char  *src;
        const size_t wantLen;
    } test;

    const test tests[] = {
        {
            .name    = "case 1: {}",
            .src     = "{}",
            .wantLen = 1,
        },
        {
            .name    = "case 2: []",
            .src     = "[]",
            .wantLen = 1,
        },
        {
            .name    = "case 2: [[]]",
            .src     = "[[]]",
            .wantLen = 2,
        },
        {
            .name    = "case 3: [[], []]",
            .src     = "[[], []]",
            .wantLen = 3,
        },
        {
            .name    = "case 4: [[], [[]], [[], []]]",
            .src     = "[[], [[]], [[], []]]",
            .wantLen = 7,
        },
        {
            .name    = "case 5: [\"a\", [[], []]]",
            .src     = "[\"a\", [[], []]]",
            .wantLen = 5,
        },
        {
            .name    = "case 6: [[], \"[], [[]]\", [[]]]",
            .src     = "[[], \"[], [[]]\", [[]]]",
            .wantLen = 5,
        },
        {
            .name    = "case 7: [1, 2, 3]",
            .src     = "[1, 2, 3]",
            .wantLen = 4,
        },
        {
            .name    = "case 7: [1, 2, [3, \"a\"], null]",
            .src     = "[1, 2, [3, \"a\"], null]",
            .wantLen = 7,
        }
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        const auto tt         = tests[i];
        JsonNode   arr[10]    = {};
        JsonNodes  dst        = {.arr = arr, .cap = sizeof(arr) / sizeof(JsonNode)};
        char       chars[128] = {};
        CharSlice  src        = CharSlice_Wrap(chars, 0, 128);
        CharSlice_WriteString(&src, tt.src);

        TEST_MESSAGE(tt.name);
        const auto r = JsonNodes_Parse(&dst, src);
        TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, r.err);
        TEST_ASSERT_EQUAL(tt.wantLen, dst.len);
    }
}

void test_non_strict(void) {
    const auto src1 = CHAR_SLICE("a: 0garbage");
    run_parse_case(src1, 2, JSON_PARSE_ERROR_INVALID, 0);

    const auto src2 = CHAR_SLICE("Day : 26\nMonth : Sep\n\nYear: 12");
    run_parse_case(src2, 6, JSON_PARSE_ERROR_INVALID, 0);

    /* nested {s don't cause a parse error. */
    const auto src3 = CHAR_SLICE("\"key {1\": 1234");
    run_parse_case(src3, 2, JSON_PARSE_ERROR_INVALID, 0);
}

void test_unmatched_brackets(void) {
    const auto src1 = CHAR_SLICE("\"key 1\": 1234}");
    run_parse_case(src1, 2, JSON_PARSE_ERROR_INVALID, 0);
    const auto src2 = CHAR_SLICE("{\"key 1\": 1234");
    run_parse_case(src2, 3, JSON_PARSE_ERROR_PARTIAL, 0);
    const auto src3 = CHAR_SLICE("{\"key 1\": 1234}}");
    run_parse_case(
        src3, 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, 0, 15, 1, JSON_NODE_TYPE_STRING, "key 1", 1,
        JSON_NODE_TYPE_PRIMITIVE, "1234"
    );
    const auto src4 = CHAR_SLICE("\"key 1\"}: 1234");
    run_parse_case(src4, 3, JSON_PARSE_ERROR_INVALID, 0);
    const auto src5 = CHAR_SLICE("{\"key {1\": 1234}");
    run_parse_case(
        src5, 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, 0, 16, 1, JSON_NODE_TYPE_STRING, "key {1", 1,
        JSON_NODE_TYPE_PRIMITIVE, "1234"
    );
    const auto src6 = CHAR_SLICE("{\"key 1\":{\"key 2\": 1234}");
    run_parse_case(src6, 5, JSON_PARSE_ERROR_PARTIAL, 5);
}

void test_object_key(void) {
    const auto src1 = CHAR_SLICE("{\"key\": 1}");
    run_parse_case(
        src1, 3, JSON_PARSE_ERROR_OK, 3, JSON_NODE_TYPE_OBJECT, 0, 10, 1, JSON_NODE_TYPE_STRING, "key", 1,
        JSON_NODE_TYPE_PRIMITIVE, "1"
    );
    const auto src2 = CHAR_SLICE("{true: 1}");
    run_parse_case(src2, 3, JSON_PARSE_ERROR_INVALID, 0);
    const auto src3 = CHAR_SLICE("{1: 1}");
    run_parse_case(src3, 3, JSON_PARSE_ERROR_INVALID, 0);
    const auto src4 = CHAR_SLICE("{{\"key\": 1}: 2}");
    run_parse_case(src4, 5, JSON_PARSE_ERROR_INVALID, 0);
    const auto src5 = CHAR_SLICE("{[1,2]: 2}");
    run_parse_case(src5, 5, JSON_PARSE_ERROR_INVALID, 0);
}

void Test_JsonParse() {
    const auto src = CHAR_SLICE(
        "{"
        "\"testStr\": \"Foo\", "
        "\"testNumber\": 1, "
        "\"testArray\": [1, 2, 3, 4], "
        "\"testObject\": { "
        "    \"prop1\": true, "
        "    \"prop2\": 1.0"
        "}"
        "}"
    );
    JsonNode  nodes[100] = {};
    JsonNodes dst        = {
               .arr = nodes,
               .cap = sizeof(nodes) / sizeof(JsonNode),
    };
    const auto r = JsonNodes_Parse(&dst, src);

    if (r.err != JSON_PARSE_ERROR_OK) {
        char msg[128];

        switch (r.err) {
            case JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED:
                sprintf(msg, "%s", "Out of memory");
                break;
            case JSON_PARSE_ERROR_INVALID:
                sprintf(msg, "%s", "Invalid input");
                break;
            case JSON_PARSE_ERROR_PARTIAL:
                sprintf(msg, "%s", "Partial input");
                break;
            default:
                sprintf(msg, "Unknown error %d", r.err);
                break;
        }

        printf("%s", msg);
    }

    for (size_t i = 0; i < sizeof(nodes) / sizeof(nodes[0]); i++) {
        const auto n = nodes[i];
        if (n.type == JSON_NODE_TYPE_NONE) {
            break;
        }

        char *typeStr;
        switch (n.type) {
            case JSON_NODE_TYPE_NONE:
                typeStr = "Undefined";
                break;
            case JSON_NODE_TYPE_OBJECT:
                typeStr = "Object";
                break;
            case JSON_NODE_TYPE_ARRAY:
                typeStr = "Array";
                break;
            case JSON_NODE_TYPE_STRING:
                typeStr = "String";
                break;
            case JSON_NODE_TYPE_PRIMITIVE:
                typeStr = "Primitive";
                break;
            default:
                typeStr = "Unknown";
        }

        const auto content = CharSlice_View(src, n.offset, n.offset + n.len);
        printf(
            "%-10s| %4zd, %4zd, %4zd| \"%.*s\"\n", typeStr, n.offset, n.len, n.childrenCount, (int)content.len,
            content.arr
        );
    }
}

void Test_JsonWrite() {
    auto      dst = CharSlice_Make(0, 1024);
    JsonStack s   = {
          .cap = 16,
          .len = 0,
          .arr = (JsonStackEntry[16]){},
    };

    CharSlice_WriteJsonStart(&dst, &s, '{');
    CharSlice_WriteJsonKey(&dst, &s, CHAR_SLICE("objField"));
    CharSlice_WriteJsonStart(&dst, &s, '{');
    CharSlice_WriteJsonKey(&dst, &s, CHAR_SLICE("numericField"));
    CharSlice_WriteJsonNumeric(&dst, &s, CHAR_SLICE("123"));
    CharSlice_WriteJsonKey(&dst, &s, CHAR_SLICE("strField"));
    CharSlice_WriteJsonString(&dst, &s, CHAR_SLICE("Foo"));
    CharSlice_WriteJsonEnd(&dst, &s);

    CharSlice_WriteJsonKey(&dst, &s, CHAR_SLICE("arrField"));
    CharSlice_WriteJsonStart(&dst, &s, '[');
    CharSlice_WriteJsonBool(&dst, &s, true);
    CharSlice_WriteJsonNumeric(&dst, &s, CHAR_SLICE("456"));
    CharSlice_WriteJsonString(&dst, &s, CHAR_SLICE("Bar"));
    CharSlice_WriteJsonEnd(&dst, &s);

    CharSlice_WriteJsonEnd(&dst, &s);

    TEST_ASSERT_EQUAL_STRING(
        "{\"objField\":{\"numericField\":123,\"strField\":\"Foo\"},\"arrField\":[true,456,\"Bar\"]}", dst.arr
    );
}

int main(void) {
    mem = Arena_Wrap(buffer);

    UNITY_BEGIN();

    RUN_TEST(test_empty);
    RUN_TEST(test_object);
    RUN_TEST(test_array);
    RUN_TEST(test_primitive);
    RUN_TEST(test_string);
    // RUN_TEST(test_partial_string);
    // RUN_TEST(test_partial_array);
    RUN_TEST(test_array_nomem);
    RUN_TEST(test_unquoted_keys);
    RUN_TEST(test_issue_22);
    RUN_TEST(test_issue_27);
    RUN_TEST(test_input_length);
    RUN_TEST(test_count);
    RUN_TEST(test_non_strict);
    RUN_TEST(test_unmatched_brackets);
    RUN_TEST(test_object_key);

    RUN_TEST(Test_JsonParse);
    RUN_TEST(Test_JsonWrite);

    printf("mem: %zd/%zd\n", mem.offset, mem.cap);
    return UNITY_END();
}
