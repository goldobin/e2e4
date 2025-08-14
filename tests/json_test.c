#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "json_reader.h"
#include "json_writer.h"

void setUp(void) {}

void tearDown(void) {}

int vtokeq(const char *s, const JsonToken *tokens, unsigned long numtok, va_list ap) {
    assert(tokens != nullptr);

    if (numtok > 0) {
        int   start, end;
        int   childrenCount = -1;
        char *value         = nullptr;

        for (unsigned long i = 0; i < numtok; i++) {
            const JsonType type = va_arg(ap, JsonType);
            if (type == JSON_TYPE_STRING) {
                value         = va_arg(ap, char *);
                childrenCount = va_arg(ap, int);
                start = end = -1;
            } else if (type == JSON_TYPE_PRIMITIVE) {
                value = va_arg(ap, char *);
                start = end = childrenCount = -1;
            } else {
                start         = va_arg(ap, int);
                end           = va_arg(ap, int);
                childrenCount = va_arg(ap, int);
                value         = nullptr;
            }
            if (tokens[i].type != type) {
                printf("token %lu type is %d, not %d\n", i, tokens[i].type, type);
                return 0;
            }
            if (start != -1 && end != -1) {
                if (tokens[i].start != start) {
                    printf("token %lu start is %zd, not %d\n", i, tokens[i].start, start);
                    return 0;
                }
                if (tokens[i].end != end) {
                    printf("token %lu end is %zd, not %d\n", i, tokens[i].end, end);
                    return 0;
                }
            }
            if (childrenCount != -1 && tokens[i].childrenCount != childrenCount) {
                printf("token %lu size is %zd, not %d\n", i, tokens[i].childrenCount, childrenCount);
                return 0;
            }

            if (s != NULL && value != NULL) {
                const char *p = s + tokens[i].start;
                if (strlen(value) != (tokens[i].end - tokens[i].start) ||
                    strncmp(p, value, tokens[i].end - tokens[i].start) != 0) {
                    printf(
                        "token %lu value is %.*s, not %s\n", i, (int)tokens[i].end - (int)tokens[i].start,
                        s + tokens[i].start, value
                    );
                    return 0;
                }
            }
        }
    }
    return 1;
}

static int tokeq(const char *s, JsonToken *tokens, unsigned long numtok, ...) {
    assert(tokens != nullptr);

    va_list args;
    va_start(args, numtok);
    const int ok = vtokeq(s, tokens, numtok, args);
    va_end(args);
    return ok;
}

int parse(const char *s, const int status, unsigned long numtok, ...) {
    int        ok = 1;
    JsonParser p;
    JsonToken *t = malloc(numtok * sizeof(JsonToken));

    JsonParser_Init(&p);
    int r = JsonParser_Parse(&p, s, strlen(s), t, numtok);
    if (r != status) {
        printf("status is %d, not %d\n", r, status);
        return 0;
    }

    if (status >= 0) {
        va_list args;
        va_start(args, numtok);
        ok = vtokeq(s, t, numtok, args);
        va_end(args);
    }
    free(t);
    return ok;
}

void test_empty() {
    TEST_ASSERT(parse("{}", 1, 1, JSON_TYPE_OBJECT, 0, 2, 0));
    TEST_ASSERT(parse("[]", 1, 1, JSON_TYPE_ARRAY, 0, 2, 0));
    TEST_ASSERT(parse("[{},{}]", 3, 3, JSON_TYPE_ARRAY, 0, 7, 2, JSON_TYPE_OBJECT, 1, 3, 0, JSON_TYPE_OBJECT, 4, 6, 0));
}

void test_object() {
    TEST_ASSERT(parse("{\"a\":0}", 3, 3, JSON_TYPE_OBJECT, 0, 7, 1, JSON_TYPE_STRING, "a", 1, JSON_TYPE_PRIMITIVE, "0")
    );
    TEST_ASSERT(parse("{\"a\":[]}", 3, 3, JSON_TYPE_OBJECT, 0, 8, 1, JSON_TYPE_STRING, "a", 1, JSON_TYPE_ARRAY, 5, 7, 0)
    );
    TEST_ASSERT(parse(
        "{\"a\":{},\"b\":{}}", 5, 5, JSON_TYPE_OBJECT, -1, -1, 2, JSON_TYPE_STRING, "a", 1, JSON_TYPE_OBJECT, -1, -1, 0,
        JSON_TYPE_STRING, "b", 1, JSON_TYPE_OBJECT, -1, -1, 0
    ));
    TEST_ASSERT(parse(
        "{\n \"Day\": 26,\n \"Month\": 9,\n \"Year\": 12\n }", 7, 7, JSON_TYPE_OBJECT, -1, -1, 3, JSON_TYPE_STRING,
        "Day", 1, JSON_TYPE_PRIMITIVE, "26", JSON_TYPE_STRING, "Month", 1, JSON_TYPE_PRIMITIVE, "9", JSON_TYPE_STRING,
        "Year", 1, JSON_TYPE_PRIMITIVE, "12"
    ));
    TEST_ASSERT(parse(
        "{\"a\": 0, \"b\": \"c\"}", 5, 5, JSON_TYPE_OBJECT, -1, -1, 2, JSON_TYPE_STRING, "a", 1, JSON_TYPE_PRIMITIVE,
        "0", JSON_TYPE_STRING, "b", 1, JSON_TYPE_STRING, "c", 0
    ));

    TEST_ASSERT(parse("{\"a\"\n0}", JSON_PARSE_ERROR_INVALID, 3));
    TEST_ASSERT(parse("{\"a\", 0}", JSON_PARSE_ERROR_INVALID, 3));
    TEST_ASSERT(parse("{\"a\": {2}}", JSON_PARSE_ERROR_INVALID, 3));
    TEST_ASSERT(parse("{\"a\": {2: 3}}", JSON_PARSE_ERROR_INVALID, 3));
    TEST_ASSERT(parse("{\"a\": {\"a\": 2 3}}", JSON_PARSE_ERROR_INVALID, 5));
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
    TEST_ASSERT(parse("[10]", 2, 2, JSON_TYPE_ARRAY, -1, -1, 1, JSON_TYPE_PRIMITIVE, "10"));
    TEST_ASSERT(parse("{\"a\": 1]", JSON_PARSE_ERROR_INVALID, 3));
    /* FIXME */
    /*TEST_ASSERT(parse("[\"a\": 1]", JSON_PARSE_ERROR_INVALID, 3));*/
}

void test_primitive() {
    TEST_ASSERT(parse(
        "{\"boolVar\" : true }", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "boolVar", 1, JSON_TYPE_PRIMITIVE,
        "true"
    ));
    TEST_ASSERT(parse(
        "{\"boolVar\" : false }", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "boolVar", 1,
        JSON_TYPE_PRIMITIVE, "false"
    ));
    TEST_ASSERT(parse(
        "{\"nullVar\" : null }", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "nullVar", 1, JSON_TYPE_PRIMITIVE,
        "null"
    ));
    TEST_ASSERT(parse(
        "{\"intVar\" : 12}", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "intVar", 1, JSON_TYPE_PRIMITIVE, "12"
    ));
    TEST_ASSERT(parse(
        "{\"floatVar\" : 12.345}", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "floatVar", 1,
        JSON_TYPE_PRIMITIVE, "12.345"
    ));
}

void test_string(void) {
    TEST_ASSERT(parse(
        "{\"strVar\" : \"hello world\"}", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "strVar", 1,
        JSON_TYPE_STRING, "hello world", 0
    ));
    TEST_ASSERT(parse(
        "{\"strVar\" : \"escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\\"}", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING,
        "strVar", 1, JSON_TYPE_STRING, "escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\", 0
    ));
    TEST_ASSERT(parse(
        "{\"strVar\": \"\"}", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "strVar", 1, JSON_TYPE_STRING, "", 0
    ));
    TEST_ASSERT(parse(
        "{\"a\":\"\\uAbcD\"}", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1, JSON_TYPE_STRING, "\\uAbcD",
        0
    ));
    TEST_ASSERT(parse(
        "{\"a\":\"str\\u0000\"}", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1, JSON_TYPE_STRING,
        "str\\u0000", 0
    ));
    TEST_ASSERT(parse(
        "{\"a\":\"\\uFFFFstr\"}", 3, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1, JSON_TYPE_STRING,
        "\\uFFFFstr", 0
    ));
    TEST_ASSERT(parse(
        "{\"a\":[\"\\u0280\"]}", 4, 4, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1, JSON_TYPE_ARRAY, -1, -1,
        1, JSON_TYPE_STRING, "\\u0280", 0
    ));

    TEST_ASSERT(parse("{\"a\":\"str\\uFFGFstr\"}", JSON_PARSE_ERROR_INVALID, 3));
    TEST_ASSERT(parse("{\"a\":\"str\\u@FfF\"}", JSON_PARSE_ERROR_INVALID, 3));
    TEST_ASSERT(parse("{{\"a\":[\"\\u028\"]}", JSON_PARSE_ERROR_INVALID, 4));
}

void test_partial_string(void) {
    JsonParser  p;
    JsonToken   tok[5];
    const char *js = "{\"x\": \"va\\\\ue\", \"y\": \"value y\"}";

    JsonParser_Init(&p);
    for (unsigned long i = 1; i <= strlen(js); i++) {
        const int r = JsonParser_Parse(&p, js, i, tok, sizeof(tok) / sizeof(tok[0]));
        if (i == strlen(js)) {
            TEST_ASSERT(r == 5);
            TEST_ASSERT(tokeq(
                js, tok, 5, JSON_TYPE_OBJECT, -1, -1, 2, JSON_TYPE_STRING, "x", 1, JSON_TYPE_STRING, "va\\\\ue", 0,
                JSON_TYPE_STRING, "y", 1, JSON_TYPE_STRING, "value y", 0
            ));
        } else {
            TEST_ASSERT(r == JSON_PARSE_ERROR_PARTIAL);
        }
    }
}

void test_partial_array(void) {
    JsonParser  p;
    JsonToken   tok[10];
    const char *js = "[ 1, true, [123, \"hello\"]]";

    JsonParser_Init(&p);
    for (unsigned long i = 1; i <= strlen(js); i++) {
        const int r = JsonParser_Parse(&p, js, i, tok, sizeof(tok) / sizeof(tok[0]));
        if (i == strlen(js)) {
            TEST_ASSERT(r == 6);
            TEST_ASSERT(tokeq(
                js, tok, 6, JSON_TYPE_ARRAY, -1, -1, 3, JSON_TYPE_PRIMITIVE, "1", JSON_TYPE_PRIMITIVE, "true",
                JSON_TYPE_ARRAY, -1, -1, 2, JSON_TYPE_PRIMITIVE, "123", JSON_TYPE_STRING, "hello", 0
            ));
        } else {
            TEST_ASSERT(r == JSON_PARSE_ERROR_PARTIAL);
        }
    }
}

void test_array_nomem(void) {
    JsonParser  p;
    JsonToken   toksmall[10], toklarge[10];
    const char *js;

    js = "  [ 1, true, [123, \"hello\"]]";

    for (int i = 0; i < 6; i++) {
        JsonParser_Init(&p);
        memset(toksmall, 0, sizeof(toksmall));
        memset(toklarge, 0, sizeof(toklarge));
        int r = JsonParser_Parse(&p, js, strlen(js), toksmall, i);
        TEST_ASSERT(r == JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED);

        memcpy(toklarge, toksmall, sizeof(toksmall));

        r = JsonParser_Parse(&p, js, strlen(js), toklarge, 10);
        TEST_ASSERT(r >= 0);
        TEST_ASSERT(tokeq(
            js, toklarge, 4, JSON_TYPE_ARRAY, -1, -1, 3, JSON_TYPE_PRIMITIVE, "1", JSON_TYPE_PRIMITIVE, "true",
            JSON_TYPE_ARRAY, -1, -1, 2, JSON_TYPE_PRIMITIVE, "123", JSON_TYPE_STRING, "hello", 0
        ));
    }
}

void test_unquoted_keys(void) {
    JsonParser  p;
    JsonToken   tok[10];
    const char *js;

    JsonParser_Init(&p);
    js = "key1: \"value\"\nkey2 : 123";

    const int r = JsonParser_Parse(&p, js, strlen(js), tok, 10);
    TEST_ASSERT(r >= 0);
    TEST_ASSERT(tokeq(
        js, tok, 4, JSON_TYPE_PRIMITIVE, "key1", JSON_TYPE_STRING, "value", 0, JSON_TYPE_PRIMITIVE, "key2",
        JSON_TYPE_PRIMITIVE, "123"
    ));
}

void test_issue_22(void) {
    JsonParser  p;
    JsonToken   tokens[128];
    const char *js;

    js = "{ \"height\":10, \"layers\":[ { \"data\":[6,6], \"height\":10, "
         "\"name\":\"Calque de Tile 1\", \"opacity\":1, \"type\":\"tilelayer\", "
         "\"visible\":true, \"width\":10, \"x\":0, \"y\":0 }], "
         "\"orientation\":\"orthogonal\", \"properties\": { }, \"tileheight\":32, "
         "\"tilesets\":[ { \"firstgid\":1, \"image\":\"..\\/images\\/tiles.png\", "
         "\"imageheight\":64, \"imagewidth\":160, \"margin\":0, "
         "\"name\":\"Tiles\", "
         "\"properties\":{}, \"spacing\":0, \"tileheight\":32, \"tilewidth\":32 "
         "}], "
         "\"tilewidth\":32, \"version\":1, \"width\":10 }";
    JsonParser_Init(&p);
    const int r = JsonParser_Parse(&p, js, strlen(js), tokens, 128);
    TEST_ASSERT(r >= 0);
}

void test_issue_27(void) {
    const char *js = "{ \"name\" : \"Jack\", \"age\" : 27 } { \"name\" : \"Anna\", ";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_PARTIAL, 8));
}

void test_input_length(void) {
    const char *js;
    int         r;
    JsonParser  p;
    JsonToken   tokens[10];

    js = "{\"a\": 0}garbage";

    JsonParser_Init(&p);
    r = JsonParser_Parse(&p, js, 8, tokens, 10);
    TEST_ASSERT(r == 3);
    TEST_ASSERT(tokeq(js, tokens, 3, JSON_TYPE_OBJECT, -1, -1, 1, JSON_TYPE_STRING, "a", 1, JSON_TYPE_PRIMITIVE, "0"));
}

void test_count(void) {
    JsonParser  p;
    const char *js;

    js = "{}";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 1);

    js = "[]";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 1);

    js = "[[]]";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 2);

    js = "[[], []]";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 3);

    js = "[[], []]";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 3);

    js = "[[], [[]], [[], []]]";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 7);

    js = "[\"a\", [[], []]]";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 5);

    js = "[[], \"[], [[]]\", [[]]]";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 5);

    js = "[1, 2, 3]";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 4);

    js = "[1, 2, [3, \"a\"], null]";
    JsonParser_Init(&p);
    TEST_ASSERT(JsonParser_Parse(&p, js, strlen(js), nullptr, 0) == 7);
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
    const char *js;
    js = "\"key 1\": 1234}";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_INVALID, 2));
    js = "{\"key 1\": 1234";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_PARTIAL, 3));
    js = "{\"key 1\": 1234}}";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_INVALID, 3));
    js = "\"key 1\"}: 1234";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_INVALID, 3));
    js = "{\"key {1\": 1234}";
    TEST_ASSERT(parse(js, 3, 3, JSON_TYPE_OBJECT, 0, 16, 1, JSON_TYPE_STRING, "key {1", 1, JSON_TYPE_PRIMITIVE, "1234")
    );
    js = "{\"key 1\":{\"key 2\": 1234}";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_PARTIAL, 5));
}

void test_object_key(void) {
    const char *js;

    js = "{\"key\": 1}";
    TEST_ASSERT(parse(js, 3, 3, JSON_TYPE_OBJECT, 0, 10, 1, JSON_TYPE_STRING, "key", 1, JSON_TYPE_PRIMITIVE, "1"));
    js = "{true: 1}";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_INVALID, 3));
    js = "{1: 1}";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_INVALID, 3));
    js = "{{\"key\": 1}: 2}";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_INVALID, 5));
    js = "{[1,2]: 2}";
    TEST_ASSERT(parse(js, JSON_PARSE_ERROR_INVALID, 5));
}

void Test_JsonParse() {
    JsonParser p = {};
    JsonParser_Init(&p);
    JsonToken tokens[100] = {};

    constexpr char in[256] =
        "{"
        "\"testStr\": \"Foo\", "
        "\"testNumber\": 1, "
        "\"testArray\": [1, 2, 3, 4], "
        "\"testObject\": { "
        "    \"prop1\": true, "
        "    \"prop2\": 1.0"
        "}"
        "}";

    const auto err = JsonParser_Parse(&p, in, sizeof(in), tokens, sizeof(tokens));

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
        strncpy(buff, in + t.start, t.end - t.start);
        printf("%-10s| %4zd, %4zd, %4zd| \"%s\"\n", typeStr, t.start, t.end, t.childrenCount, buff);
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
    UNITY_BEGIN();
    RUN_TEST(Test_JsonParse);
    RUN_TEST(test_empty);
    RUN_TEST(test_object);
    RUN_TEST(test_array);
    RUN_TEST(test_primitive);
    RUN_TEST(test_string);
    RUN_TEST(test_partial_string);
    RUN_TEST(test_partial_array);
    RUN_TEST(test_array_nomem);
    // RUN_TEST(test_unquoted_keys);
    RUN_TEST(test_issue_22);
    RUN_TEST(test_issue_27);
    RUN_TEST(test_input_length);
    RUN_TEST(test_count);
    // RUN_TEST(test_nonstrict);
    RUN_TEST(test_unmatched_brackets);
    RUN_TEST(test_object_key);

    RUN_TEST(Test_JsonWrite);
    return UNITY_END();
}
