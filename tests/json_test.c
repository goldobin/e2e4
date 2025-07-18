#import <jsmn.h>
#include <string.h>
#import <unity.h>

#include "jsw.h"

void setUp(void) {}

void tearDown(void) {}

void Test_JsonParse() {
    jsmn_parser p = {};
    jsmn_init(&p);

    jsmntok_t tokens[100] = {};

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

    const auto err = jsmn_parse(&p, in, sizeof(in), tokens, sizeof(tokens));

    if (err < 0) {
        char* msg;

        switch (err) {
            case JSMN_ERROR_NOMEM:
                msg = "Out of memory";
                break;
            case JSMN_ERROR_INVAL:
                msg = "Invalid input";
                break;
            case JSMN_ERROR_PART:
                msg = "Partial input";
                break;
            default:
                char msg1[128] = {};
                snprintf(msg1, sizeof(msg1), "Unknown error %d", err);
                msg = msg1;
                break;
        }

        printf(msg);
    }

    for (size_t i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++) {
        const auto t = tokens[i];
        if (t.type == JSMN_UNDEFINED) {
            break;
        }

        char* typeStr;
        switch (t.type) {
            case JSMN_UNDEFINED:
                typeStr = "Undefined";
                break;
            case JSMN_OBJECT:
                typeStr = "Object";
                break;
            case JSMN_ARRAY:
                typeStr = "Array";
                break;
            case JSMN_STRING:
                typeStr = "String";
                break;
            case JSMN_PRIMITIVE:
                typeStr = "Primitive";
                break;
        }

        char buff[256] = {};
        strncpy(buff, in + t.start, t.end - t.start);
        printf("%-10s| %4d, %4d, %4d| \"%s\"\n", typeStr, t.start, t.end, t.size, buff);
    }
}

void Test_JsonWrite() {
    const auto dst = CharSlice_Make(0, 1024);

    Jsw jsw = {
        .stack =
            {
                .cap = 16,
                .len = 0,
                .arr = (JswStateType[16]){},
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
    RUN_TEST(Test_JsonWrite);
    return UNITY_END();
}
