#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "arena.h"
#include "json_reader.h"
#include "json_writer.h"

uint8_t buffer[1024 * 1024] = {};
Arena   mem                 = {};

void setUp(void) {}

void tearDown(void) {}

int vtokeq(const char *s, const JsonTokens *ts, va_list ap) {
    assert(ts != nullptr);

    if (ts->len < 1) {
        return 1;
    }

    for (size_t i = 0; i < ts->len; i++) {
        size_t offset        = 0;
        size_t len           = 0;
        size_t childrenCount = 0;
        char  *value         = nullptr;

        const JsonType type = va_arg(ap, JsonType);
        if (type == JSON_TYPE_STRING) {
            value         = va_arg(ap, char *);
            childrenCount = va_arg(ap, int);
        } else if (type == JSON_TYPE_PRIMITIVE) {
            value = va_arg(ap, char *);
        } else {
            offset        = va_arg(ap, int);
            const int end = va_arg(ap, int);
            len           = end - offset;
            childrenCount = va_arg(ap, int);
        }

        const auto t = JsonTokens_At(ts, i);
        if (t->type != type) {
            printf("token %lu type is %d, not %d\n", i, t->type, type);
            return 0;
        }

        if (len != 0) {
            if (t->offset != offset) {
                printf("token %lu offset is %zd, not %zd\n", i, t->offset, offset);
                return 0;
            }

            if (t->len != len) {
                printf("token %lu len is %zd, not %zd\n", i, t->len, len);
                return 0;
            }
        }

        if (t->childrenCount != childrenCount) {
            printf("token %lu size is %zd, not %zd\n", i, t->childrenCount, childrenCount);
            return 0;
        }

        if (s != nullptr && value != nullptr) {
            const char *p = s + t->offset;
            if (strlen(value) != t->len || strncmp(p, value, t->len) != 0) {
                printf("token %lu value is %.*s, not %s\n", i, (int)t->len, s + t->offset, value);
                return 0;
            }
        }
    }

    return 1;
}

static int tokeq(const char *s, JsonTokens *ts, ...) {
    assert(ts != nullptr);

    va_list args;
    va_start(args, numtok);
    const int ok = vtokeq(s, ts, args);
    va_end(args);
    return ok;
}

int parse(const char *s, const size_t poolSize, const JsonParseErr wantErr, const size_t wantTokenCount, ...) {
    int        ok = 1;
    JsonParser p;
    JsonTokens ts = {
        .arr = Arena_Alloc(&mem, poolSize * sizeof(JsonToken)),
        .cap = poolSize,
    };

    JsonParser_Init(&p);
    const int err = JsonParser_Parse(&p, &ts, s, strlen(s));
    if (err != wantErr) {
        printf("status is %d, not %d\n", err, wantErr);
        return 0;
    }

    if (wantErr != JSON_PARSE_ERROR_OK) {
        return 1;
    }

    if (wantTokenCount != ts.len) {
        printf("token len is %zd, not %zd\n", ts.len, wantTokenCount);
        return 0;
    }

    if (wantTokenCount > 0) {
        va_list args;
        va_start(args, wantTokensCount);
        ok = vtokeq(s, &ts, args);
        va_end(args);
    }

    return ok;
}

void test_empty() {
    TEST_ASSERT(parse("{}", 1, JSON_PARSE_ERROR_OK, 1, JSON_TYPE_OBJECT, 0, 2, 0));
    TEST_ASSERT(parse("[]", 1, JSON_PARSE_ERROR_OK, 1, JSON_TYPE_ARRAY, 0, 2, 0));
    TEST_ASSERT(parse(
        "[{},{}]", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_ARRAY, 0, 7, 2, JSON_TYPE_OBJECT, 1, 3, 0, JSON_TYPE_OBJECT, 4,
        6, 0
    ));
}

void test_object() {
    TEST_ASSERT(parse(
        "{\"a\":0}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, 0, 7, 1, JSON_TYPE_STRING, "a", 1,
        JSON_TYPE_PRIMITIVE, "0"
    ));
    TEST_ASSERT(parse(
        "{\"a\":[]}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, 0, 8, 1, JSON_TYPE_STRING, "a", 1, JSON_TYPE_ARRAY,
        5, 7, 0
    ));
    TEST_ASSERT(parse(
        "{\"a\":{},\"b\":{}}", 5, JSON_PARSE_ERROR_OK, 5, JSON_TYPE_OBJECT, -1, -1, 2, JSON_TYPE_STRING, "a", 1,
        JSON_TYPE_OBJECT, -1, -1, 0, JSON_TYPE_STRING, "b", 1, JSON_TYPE_OBJECT, -1, -1, 0
    ));
    TEST_ASSERT(parse(
        "{\n \"Day\": 26,\n \"Month\": 9,\n \"Year\": 12\n }", 7, JSON_PARSE_ERROR_OK, 7, JSON_TYPE_OBJECT, -1, -1, 3,
        JSON_TYPE_STRING, "Day", 1, JSON_TYPE_PRIMITIVE, "26", JSON_TYPE_STRING, "Month", 1, JSON_TYPE_PRIMITIVE, "9",
        JSON_TYPE_STRING, "Year", 1, JSON_TYPE_PRIMITIVE, "12"
    ));
    TEST_ASSERT(parse(
        "{\"a\": 0, \"b\": \"c\"}", 5, JSON_PARSE_ERROR_OK, 5, JSON_TYPE_OBJECT, -1, -1, 2, JSON_TYPE_STRING, "a", 1,
        JSON_TYPE_PRIMITIVE, "0", JSON_TYPE_STRING, "b", 1, JSON_TYPE_STRING, "c", 0
    ));

    TEST_ASSERT(parse("{\"a\"\n0}", 3, JSON_PARSE_ERROR_INVALID, 0));
    TEST_ASSERT(parse("{\"a\", 0}", 3, JSON_PARSE_ERROR_INVALID, 0));
    TEST_ASSERT(parse("{\"a\": {2}}", 3, JSON_PARSE_ERROR_INVALID, 0));
    TEST_ASSERT(parse("{\"a\": {2: 3}}", 3, JSON_PARSE_ERROR_INVALID, 0));
    TEST_ASSERT(parse("{\"a\": {\"a\": 2 3}}", 5, JSON_PARSE_ERROR_INVALID, 0));
    /* FIXME */
    /*TEST_ASSERT(parse("{\"a\"}", JSON_PARSE_ERROR_INVALID, 2));*/
    /*TEST_ASSERT(parse("{\"a\": 1, \"b\"}", JSON_PARSE_ERROR_INVALID, 4));*/
    /*TEST_ASSERT(parse("{\"a\",\"b\":1}", JSON_PARSE_ERROR_INVALID, 4));*/
    /*TEST_ASSERT(parse("{\"a\":1,}", JSON_PARSE_ERROR_INVALID, 4));*/
    /*TEST_ASSERT(parse("{\"a\":\"b\":\"c\"}", JSON_PARSE_ERROR_INVALID, 4));*/
    /*TEST_ASSERT(parse("{,}", JSON_PARSE_ERROR_INVALID, 4));*/
}

void test_array(void) {
    /* FIXME */
    /*TEST_ASSERT(parse("[10}", JSON_PARSE_ERROR_INVALID, 3));*/
    /*TEST_ASSERT(parse("[1,,3]", JSON_PARSE_ERROR_INVALID, 3)*/
    TEST_ASSERT(parse("[10]", 2, JSON_PARSE_ERROR_OK, 2, JSON_TYPE_ARRAY, -1, -1, 1, JSON_TYPE_PRIMITIVE, "10"));
    TEST_ASSERT(parse("{\"a\": 1]", 3, JSON_PARSE_ERROR_INVALID, 0));
    /* FIXME */
    /*TEST_ASSERT(parse("[\"a\": 1]", JSON_PARSE_ERROR_INVALID, 3));*/
}

void test_primitive() {
    TEST_ASSERT(parse(
        "{\"boolVar\" : true }", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "boolVar", 1,
        JSON_TYPE_PRIMITIVE, "true"
    ));
    TEST_ASSERT(parse(
        "{\"boolVar\" : false }", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "boolVar",
        1, JSON_TYPE_PRIMITIVE, "false"
    ));
    TEST_ASSERT(parse(
        "{\"nullVar\" : null }", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "nullVar", 1,
        JSON_TYPE_PRIMITIVE, "null"
    ));
    TEST_ASSERT(parse(
        "{\"intVar\" : 12}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "intVar", 1,
        JSON_TYPE_PRIMITIVE, "12"
    ));
    TEST_ASSERT(parse(
        "{\"floatVar\" : 12.345}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "floatVar",
        1, JSON_TYPE_PRIMITIVE, "12.345"
    ));
}

void test_string(void) {
    // TEST_ASSERT(parse(
    //     "{\"strVar\" : \"hello world\"}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING,
    //     "strVar", 1, JSON_TYPE_STRING, "hello world", 0
    // ));
    // TEST_ASSERT(parse(
    //     "{\"strVar\" : \"escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\\"}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1,
    //     -1, 1, JSON_TYPE_STRING, "strVar", 1, JSON_TYPE_STRING, "escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\", 0
    // ));
    TEST_ASSERT(parse(
        "{\"strVar\": \"\"}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "strVar", 1,
        JSON_TYPE_STRING, "", 0
    ));
    // TEST_ASSERT(parse(
    //     "{\"a\":\"\\uAbcD\"}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1,
    //     JSON_TYPE_STRING, "\\uAbcD", 0
    // ));
    // TEST_ASSERT(parse(
    //     "{\"a\":\"str\\u0000\"}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1,
    //     JSON_TYPE_STRING, "str\\u0000", 0
    // ));
    // TEST_ASSERT(parse(
    //     "{\"a\":\"\\uFFFFstr\"}", 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1,
    //     JSON_TYPE_STRING, "\\uFFFFstr", 0
    // ));
    // TEST_ASSERT(parse(
    //     "{\"a\":[\"\\u0280\"]}", 4, JSON_PARSE_ERROR_OK, 4, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1,
    //     JSON_TYPE_ARRAY, -1, -1, 1, JSON_TYPE_STRING, "\\u0280", 0
    // ));

    // TEST_ASSERT(parse("{\"a\":\"str\\uFFGFstr\"}", 3, JSON_PARSE_ERROR_INVALID, 0));
    // TEST_ASSERT(parse("{\"a\":\"str\\u@FfF\"}", 3, JSON_PARSE_ERROR_INVALID, 0));
    // TEST_ASSERT(parse("{{\"a\":[\"\\u028\"]}", 4, JSON_PARSE_ERROR_INVALID, 0));
}

void test_partial_string(void) {
    const char *src = "{\"x\": \"va\\\\ue\", \"y\": \"value y\"}";
    JsonToken   arr[5];
    JsonTokens  dst = {
         .arr = arr,
         .cap = sizeof(arr) / sizeof(JsonToken),
    };

    JsonParser p;
    JsonParser_Init(&p);
    for (unsigned long i = 1; i <= strlen(src); i++) {
        const JsonParseErr err = JsonParser_Parse(&p, &dst, src, i);
        if (i != strlen(src)) {
            TEST_ASSERT(err == JSON_PARSE_ERROR_PARTIAL);
            continue;
        }

        TEST_ASSERT(err == JSON_PARSE_ERROR_OK);
        TEST_ASSERT_EQUAL(5, dst.len);
        TEST_ASSERT(tokeq(
            src, &dst, JSON_TYPE_OBJECT, -1, -1, 2, JSON_TYPE_STRING, "x", 1, JSON_TYPE_STRING, "va\\\\ue", 0,
            JSON_TYPE_STRING, "y", 1, JSON_TYPE_STRING, "value y", 0
        ));
    }
}

void test_partial_array(void) {
    const char *src = "[ 1, true, [123, \"hello\"]]";

    JsonToken  arr[10];
    JsonTokens dst = {
        .arr = arr,
        .cap = sizeof(arr) / sizeof(JsonToken),
    };

    JsonParser p;
    JsonParser_Init(&p);
    for (size_t i = 1; i <= strlen(src); i++) {
        const JsonParseErr err = JsonParser_Parse(&p, &dst, src, i);
        if (i != strlen(src)) {
            TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_PARTIAL, err);
            continue;
        }

        TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, err);
        TEST_ASSERT_EQUAL(6, dst.len);
        TEST_ASSERT(tokeq(
            src, &dst, JSON_TYPE_ARRAY, -1, -1, 3, JSON_TYPE_PRIMITIVE, "1", JSON_TYPE_PRIMITIVE, "true",
            JSON_TYPE_ARRAY, -1, -1, 2, JSON_TYPE_PRIMITIVE, "123", JSON_TYPE_STRING, "hello", 0
        ));
    }
}

void test_array_nomem(void) {
    for (int i = 0; i < 6; i++) {
        const char *src = "  [ 1, true, [123, \"hello\"]]";
        JsonParser  p   = {};
        JsonParser_Init(&p);

        constexpr size_t arrLen      = 10;
        JsonToken        arr[arrLen] = {};

        JsonTokens         dstSmall = {.arr = arr, .cap = i};
        const JsonParseErr err1     = JsonParser_Parse(&p, &dstSmall, src, strlen(src));
        TEST_ASSERT(err1 == JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED);

        JsonTokens         dstLarge = {.arr = arr, .len = dstSmall.len, .cap = arrLen};
        const JsonParseErr err2     = JsonParser_Parse(&p, &dstLarge, src, strlen(src));
        TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, err2);
        TEST_ASSERT(tokeq(
            src, &dstLarge, JSON_TYPE_ARRAY, -1, -1, 3, JSON_TYPE_PRIMITIVE, "1", JSON_TYPE_PRIMITIVE, "true",
            JSON_TYPE_ARRAY, -1, -1, 2, JSON_TYPE_PRIMITIVE, "123", JSON_TYPE_STRING, "hello", 0
        ));
    }
}

void test_unquoted_keys(void) {
    const char *src = "key1: \"value\"\nkey2 : 123";

    JsonToken  arr[10];
    JsonTokens dst = {.arr = arr, .cap = sizeof(arr) / sizeof(JsonToken)};
    JsonParser p;
    JsonParser_Init(&p);

    const JsonParseErr err = JsonParser_Parse(&p, &dst, src, strlen(src));
    TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_INVALID, err);
    TEST_ASSERT(tokeq(
        src, &dst, JSON_TYPE_PRIMITIVE, "key1", JSON_TYPE_STRING, "value", 0, JSON_TYPE_PRIMITIVE, "key2",
        JSON_TYPE_PRIMITIVE, "123"
    ));
}

void test_issue_22(void) {
    const char *src =
        "{ \"height\":10, \"layers\":[ { \"data\":[6,6], \"height\":10, "
        "\"name\":\"Calque de Tile 1\", \"opacity\":1, \"type\":\"tilelayer\", "
        "\"visible\":true, \"width\":10, \"x\":0, \"y\":0 }], "
        "\"orientation\":\"orthogonal\", \"properties\": { }, \"tileheight\":32, "
        "\"tilesets\":[ { \"firstgid\":1, \"image\":\"..\\/images\\/tiles.png\", "
        "\"imageheight\":64, \"imagewidth\":160, \"margin\":0, "
        "\"name\":\"Tiles\", "
        "\"properties\":{}, \"spacing\":0, \"tileheight\":32, \"tilewidth\":32 "
        "}], "
        "\"tilewidth\":32, \"version\":1, \"width\":10 }";
    JsonToken  arr[128] = {};
    JsonTokens dst      = {.arr = arr, .cap = sizeof(arr) / sizeof(JsonToken)};
    JsonParser p;
    JsonParser_Init(&p);
    const JsonParseErr err = JsonParser_Parse(&p, &dst, src, strlen(src));
    TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, err);
}

void test_issue_27(void) {
    const char *src = "{ \"name\" : \"Jack\", \"age\" : 27 } { \"name\" : \"Anna\", ";
    TEST_ASSERT(parse(src, 8, JSON_PARSE_ERROR_PARTIAL, 0));
}

void test_input_length(void) {
    const char *src = "{\"a\": 0}garbage";

    JsonToken  arr[10] = {};
    JsonTokens dst     = {
            .arr = arr,
            .cap = sizeof(arr) / sizeof(JsonToken),
    };

    JsonParser p;
    JsonParser_Init(&p);
    const JsonParseErr err = JsonParser_Parse(&p, &dst, src, strlen(src));
    TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_INVALID, err);
    TEST_ASSERT_EQUAL(3, dst.len);
    TEST_ASSERT(tokeq(src, &dst, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1, JSON_TYPE_PRIMITIVE, "0"));
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
        JsonToken  arr[10] = {};
        JsonTokens dst     = {.arr = arr, .cap = sizeof(arr) / sizeof(JsonToken)};
        JsonParser p;
        JsonParser_Init(&p);

        const auto tt = tests[i];
        TEST_MESSAGE(tt.name);
        const auto err = JsonParser_Parse(&p, &dst, tt.src, strlen(tt.src));
        TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, err);
        TEST_ASSERT_EQUAL(tt.wantLen, dst.len);
    }
}

void test_nonstrict(void) {
    const char *js;
    js = "a: 0garbage";
    TEST_ASSERT(parse(js, 2, 2, JSON_TYPE_PRIMITIVE, "a", JSON_TYPE_PRIMITIVE, "0garbage"));

    js = "Day : 26\nMonth : Sep\n\nYear: 12";
    TEST_ASSERT(parse(
        js, 6, 6, JSON_TYPE_PRIMITIVE, "Day", JSON_TYPE_PRIMITIVE, "26", JSON_TYPE_PRIMITIVE, "Month",
        JSON_TYPE_PRIMITIVE, "Sep", JSON_TYPE_PRIMITIVE, "Year", JSON_TYPE_PRIMITIVE, "12"
    ));

    /* nested {s don't cause a parse error. */
    js = "\"key {1\": 1234";
    TEST_ASSERT(parse(js, 2, 2, JSON_TYPE_STRING, "key {1", 1, JSON_TYPE_PRIMITIVE, "1234"));
}

void test_unmatched_brackets(void) {
    const char *src = "\"key 1\": 1234}";
    TEST_ASSERT(parse(src, 2, JSON_PARSE_ERROR_INVALID, 0));
    src = "{\"key 1\": 1234";
    TEST_ASSERT(parse(src, 3, JSON_PARSE_ERROR_PARTIAL, 0));
    src = "{\"key 1\": 1234}}";
    TEST_ASSERT(parse(src, 3, JSON_PARSE_ERROR_INVALID, 0));
    src = "\"key 1\"}: 1234";
    TEST_ASSERT(parse(src, 3, JSON_PARSE_ERROR_INVALID, 0));
    src = "{\"key {1\": 1234}";
    TEST_ASSERT(parse(
        src, 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, 0, 16, 1, JSON_TYPE_STRING, "key {1", 1, JSON_TYPE_PRIMITIVE,
        "1234"
    ));
    src = "{\"key 1\":{\"key 2\": 1234}";
    TEST_ASSERT(parse(src, 5, JSON_PARSE_ERROR_PARTIAL, 5));
}

void test_object_key(void) {
    const char *src = "{\"key\": 1}";
    TEST_ASSERT(parse(
        src, 3, JSON_PARSE_ERROR_OK, 3, JSON_TYPE_OBJECT, 0, 10, 1, JSON_TYPE_STRING, "key", 1, JSON_TYPE_PRIMITIVE, "1"
    ));
    src = "{true: 1}";
    TEST_ASSERT(parse(src, 3, JSON_PARSE_ERROR_INVALID, 0));
    src = "{1: 1}";
    TEST_ASSERT(parse(src, 3, JSON_PARSE_ERROR_INVALID, 0));
    src = "{{\"key\": 1}: 2}";
    TEST_ASSERT(parse(src, 5, JSON_PARSE_ERROR_INVALID, 0));
    src = "{[1,2]: 2}";
    TEST_ASSERT(parse(src, 5, JSON_PARSE_ERROR_INVALID, 0));
}

void Test_JsonParse() {
    JsonParser p = {};
    JsonParser_Init(&p);
    JsonToken  tokens[100] = {};
    JsonTokens dst         = {
                .arr = tokens,
                .cap = sizeof(tokens) / sizeof(JsonToken),
    };

    constexpr char src[256] =
        "{"
        "\"testStr\": \"Foo\", "
        "\"testNumber\": 1, "
        "\"testArray\": [1, 2, 3, 4], "
        "\"testObject\": { "
        "    \"prop1\": true, "
        "    \"prop2\": 1.0"
        "}"
        "}";

    const auto err = JsonParser_Parse(&p, &dst, src, sizeof(src));

    if (err < 0) {
        char msg[128];

        switch (err) {
            case JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED:
                sprintf(msg, "%s", "Out of memory");
                break;
            case JSON_PARSE_ERROR_INVALID:
                sprintf(msg, "%s", "Invalid input");
                break;
            case JSON_PARSE_ERROR_PARTIAL:
                sprintf(msg, "%s", "Partial input");
                break;
            default:
                sprintf(msg, "Unknown error %d", err);
                break;
        }

        printf("%s", msg);
    }

    for (size_t i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++) {
        const auto t = tokens[i];
        if (t.type == JSON_TYPE_UNDEFINED) {
            break;
        }

        char *typeStr;
        switch (t.type) {
            case JSON_TYPE_UNDEFINED:
                typeStr = "Undefined";
                break;
            case JSON_TYPE_OBJECT:
                typeStr = "Object";
                break;
            case JSON_TYPE_ARRAY:
                typeStr = "Array";
                break;
            case JSON_TYPE_STRING:
                typeStr = "String";
                break;
            case JSON_TYPE_PRIMITIVE:
                typeStr = "Primitive";
                break;
            default:
                typeStr = "Unknown";
        }

        char buff[256] = {};
        strncpy(buff, src + t.offset, t.len);
        printf("%-10s| %4zd, %4zd, %4zd| \"%s\"\n", typeStr, t.offset, t.len, t.childrenCount, buff);
    }
}

void Test_JsonWrite() {
    const auto dst = CharSlice_Make(0, 1024);

    Jsw jsw = {
        .stack =
            {
                .cap = 16,
                .len = 0,
                .arr = (JswState[16]){},
            },
        .dst = dst
    };

    Jsw_Object(&jsw);

    Jsw_Field(&jsw, CHAR_SLICE("objField"));
    Jsw_Object(&jsw);
    Jsw_Field(&jsw, CHAR_SLICE("numericField"));
    Jsw_Numeric(&jsw, 123);
    Jsw_Field(&jsw, CHAR_SLICE("strField"));
    Jsw_String(&jsw, CHAR_SLICE("Foo"));
    Jsw_End(&jsw);

    Jsw_Field(&jsw, CHAR_SLICE("arrField"));
    Jsw_Array(&jsw);
    Jsw_Bool(&jsw, true);
    Jsw_Numeric(&jsw, 123);
    Jsw_String(&jsw, CHAR_SLICE("Bar"));
    Jsw_End(&jsw);

    Jsw_End(&jsw);

    TEST_ASSERT_EQUAL_STRING(
        "{\"objField\":{\"numericField\":123.000000,\"strField\":\"Foo\"},\"arrField\":[true,123.000000,\"Bar\"]}",
        dst.arr
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
    RUN_TEST(test_partial_string);
    RUN_TEST(test_partial_array);
    RUN_TEST(test_array_nomem);
    RUN_TEST(test_unquoted_keys);
    RUN_TEST(test_issue_22);
    RUN_TEST(test_issue_27);
    RUN_TEST(test_input_length);
    RUN_TEST(test_count);
    // RUN_TEST(test_nonstrict);
    RUN_TEST(test_unmatched_brackets);
    RUN_TEST(test_object_key);

    RUN_TEST(Test_JsonParse);
    RUN_TEST(Test_JsonWrite);

    printf("mem: %zd/%zd\n", mem.offset, mem.cap);
    return UNITY_END();
}
