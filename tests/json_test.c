#include "json.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "arena.h"

uint8_t buffer[1024 * 32] = {};
Arena   mem               = {};

void setUp() {}
void tearDown() {}

typedef struct {
    Str value;
} PrimitiveMatchingRule;

typedef struct {
    Str    value;
    size_t childrenCount;
} StringMatchingRule;

typedef struct {
    size_t offset;
    size_t len;
    size_t childrenCount;
} OtherMatchingRule;

typedef struct {
    JsonNodeType type;
    union {
        PrimitiveMatchingRule primitive;
        StringMatchingRule    string;
        OtherMatchingRule     other;
    };
} NodeMatchingRule;

constexpr size_t         NODE_MATCH_RULES_CAP = 16;
typedef NodeMatchingRule NodeMatchingRules[NODE_MATCH_RULES_CAP];

bool NodeMatchingRule_Empty(const NodeMatchingRule r) { return r.type == JSON_NODE_TYPE_UNSPECIFIED; }

void NodeMatchingRules_AssertMatches(const NodeMatchingRules rules, const Str src, const JsonNodes nodes) {
    assert(nodes.len <= NODE_MATCH_RULES_CAP);
    if (nodes.len < 1) {
        return;
    }

    for (size_t i = 0; i < nodes.len; i++) {
        const auto rule = rules[i];
        if (NodeMatchingRule_Empty(rule)) {
            continue;
        }
        const auto n = JsonNodes_At(nodes, i);
        TEST_ASSERT_EQUAL_MESSAGE(rule.type, n->type, "node type doesn't match");

        if (rule.type == JSON_NODE_TYPE_STRING) {
            const auto v = Str_View(src, n->offset, n->offset + n->len);
            TEST_ASSERT_TRUE_MESSAGE(Str_Equals(rule.string.value, v), "string content doesn't match");
            TEST_ASSERT_EQUAL_MESSAGE(
                rule.string.childrenCount, n->childrenCount, "string children count doesn't match"
            );
            continue;
        }

        if (rule.type == JSON_NODE_TYPE_PRIMITIVE) {
            const auto v = Str_View(src, n->offset, n->offset + n->len);
            TEST_ASSERT_TRUE_MESSAGE(Str_Equals(rule.primitive.value, v), "primitive content doesn't match");
            continue;
        }

        TEST_ASSERT_EQUAL_MESSAGE(rule.other.childrenCount, n->childrenCount, "children doesn't match");
        if (rule.other.offset == 0 && rule.other.len == 0) {
            continue;
        }

        TEST_ASSERT_EQUAL_MESSAGE(rule.other.offset, n->offset, "offset doesn't match");
        TEST_ASSERT_EQUAL_MESSAGE(rule.other.len, n->len, "len doesn't match");
    }
}

typedef struct {
    const Str               src;
    const size_t            poolSize;
    const JsonParseErr      wantErr;
    const size_t            wantNodeCount;
    const NodeMatchingRules rules;
} ParseTest;

void ParseTest_Run(const ParseTest tt) {
    JsonNodes nodes = {
        .arr = Arena_Alloc(&mem, tt.poolSize * sizeof(JsonNode)),
        .cap = tt.poolSize,
    };
    const auto r = JsonNodes_Parse(&nodes, tt.src);
    TEST_ASSERT_EQUAL_MESSAGE(tt.wantErr, r.err, "error doesn't match");
    if (tt.wantErr != JSON_PARSE_ERROR_OK) {
        return;
    }

    TEST_ASSERT_EQUAL_MESSAGE(nodes.len, tt.wantNodeCount, "node count doesn't match");
    if (tt.wantNodeCount == 0) {
        return;
    }
    NodeMatchingRules_AssertMatches(tt.rules, tt.src, nodes);
}

void Test_Empty() {
    const ParseTest tests[] = {
        {
            .src           = CHAR_SLICE("{}"),
            .poolSize      = 1,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 1,
            .rules         = {{.type = JSON_NODE_TYPE_OBJECT, .other = {.offset = 0, .len = 2, .childrenCount = 0}}},
        },
        {
            .src           = CHAR_SLICE("[]"),
            .poolSize      = 1,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 1,
            .rules         = {{.type = JSON_NODE_TYPE_ARRAY, .other = {.offset = 0, .len = 2, .childrenCount = 0}}},
        },
        {
            .src           = CHAR_SLICE("[{},{}]"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules         = {
                {.type = JSON_NODE_TYPE_ARRAY, .other = {.offset = 0, .len = 7, .childrenCount = 2}},
                {.type = JSON_NODE_TYPE_OBJECT, .other = {.offset = 1, .len = 2, .childrenCount = 0}},
                {.type = JSON_NODE_TYPE_OBJECT, .other = {.offset = 4, .len = 2, .childrenCount = 0}},
            },
        }
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(ParseTest); i++) {
        const auto tt = tests[i];
        TEST_MESSAGE(tt.src.arr);
        ParseTest_Run(tt);
    }
}

void Test_Object() {
    const ParseTest tests[] = {
        {
            .src           = CHAR_SLICE("{\"a\":0}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.offset = 0, .len = 7, .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("a"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("0")}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"a\":[]}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.offset = 0, .len = 8, .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("a"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_ARRAY, .other = {.offset = 5, .len = 2, .childrenCount = 0}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"a\":{},\"b\":{}}"),
            .poolSize      = 5,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 5,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.offset = 0, .len = 15, .childrenCount = 2}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("a"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 0}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("b"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 0}},
                },
        },
        {
            .src           = CHAR_SLICE("{\n \"Day\": 26,\n \"Month\": 9,\n \"Year\": 12\n }"),
            .poolSize      = 7,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 7,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 3}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("Day"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("26")}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("Month"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("9")}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("Year"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("12")}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"a\": 0, \"b\": \"c\"}"),
            .poolSize      = 5,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 5,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 2}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("a"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("0")}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("b"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("c"), .childrenCount = 0}},
                },
        },
        {
            .src      = CHAR_SLICE("{\"a\"\n0}"),
            .poolSize = 3,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("{\"a\", 0}"),
            .poolSize = 3,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("{\"a\": {2: 3}}"),
            .poolSize = 3,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("{\"a\": {\"a\": 2 3}}"),
            .poolSize = 5,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        // FIXME
        // {
        //     .src      = CHAR_SLICE("{\"a\"}"),
        //     .poolSize = 2,
        //     .wantErr  = JSON_PARSE_ERROR_INVALID,
        // },
        // {
        //     .src      = CHAR_SLICE("{\"a\": 1, \"b\"}"),
        //     .poolSize = 7,
        //     .wantErr  = JSON_PARSE_ERROR_INVALID,
        // },
        // {
        //     .src      = CHAR_SLICE("{\"a\",\"b\":1}"),
        //     .poolSize = 7,
        //     .wantErr  = JSON_PARSE_ERROR_INVALID,
        // },
        // {
        //     .src      = CHAR_SLICE("{\"a\":1,}"),
        //     .poolSize = 7,
        //     .wantErr  = JSON_PARSE_ERROR_INVALID,
        // },
        // {
        //     .src      = CHAR_SLICE("{\"a\":\"b\":\"c\"}"),
        //     .poolSize = 7,
        //     .wantErr  = JSON_PARSE_ERROR_INVALID,
        // },
        // {
        //     .src      = CHAR_SLICE("{,}"),
        //     .poolSize = 7,
        //     .wantErr  = JSON_PARSE_ERROR_INVALID,
        // },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(ParseTest); i++) {
        const auto tt = tests[i];
        TEST_MESSAGE(tt.src.arr);
        ParseTest_Run(tt);
    }
}

void test_array(void) {
    const ParseTest tests[] = {
        {.src           = CHAR_SLICE("[10]"),
         .poolSize      = 2,
         .wantErr       = JSON_PARSE_ERROR_OK,
         .wantNodeCount = 2,
         .rules =
             {
                 {.type = JSON_NODE_TYPE_ARRAY, .other = {.childrenCount = 1}},
                 {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("10")}},
             }},
        {
            .src      = CHAR_SLICE("{\"a\": 1]"),
            .poolSize = 3,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("[10}"),
            .poolSize = 2,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        // FIXME
        //  {
        //      .src      = CHAR_SLICE("[1,,3]"),
        //      .poolSize = 4,
        //      .wantErr  = JSON_PARSE_ERROR_INVALID,
        //  }
        //  {
        //      .src      = CHAR_SLICE("[\"a\": 1]"),
        //      .poolSize = 3,
        //      .wantErr  = JSON_PARSE_ERROR_INVALID,
        //  },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(ParseTest); i++) {
        const auto tt = tests[i];
        TEST_MESSAGE(tt.src.arr);
        ParseTest_Run(tt);
    }
}

void Test_Primitive() {
    const ParseTest tests[] = {
        {.src           = CHAR_SLICE("{\"boolVar\" : true }"),
         .poolSize      = 3,
         .wantErr       = JSON_PARSE_ERROR_OK,
         .wantNodeCount = 3,
         .rules =
             {
                 {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                 {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("boolVar"), .childrenCount = 1}},
                 {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("true")}},
             }},
        {
            .src           = CHAR_SLICE("{\"boolVar\" : false }"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("boolVar"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("false")}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"nullVar\" : null }"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("nullVar"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("null")}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"intVar\" : 12 }"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("intVar"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("12")}},
                },
        },
        {.src           = CHAR_SLICE("{\"floatVar\" : 12.345 }"),
         .poolSize      = 3,
         .wantErr       = JSON_PARSE_ERROR_OK,
         .wantNodeCount = 3,
         .rules         = {
             {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
             {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("floatVar"), .childrenCount = 1}},
             {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("12.345")}},
         }},
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(ParseTest); i++) {
        const auto tt = tests[i];
        TEST_MESSAGE(tt.src.arr);
        ParseTest_Run(tt);
    }
}

void Test_String(void) {
    const ParseTest tests[] = {
        {
            .src           = CHAR_SLICE("{\"strVar\" : \"hello world\"}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("strVar"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("hello world"), .childrenCount = 0}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"strVar\" : \"escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\\"}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("strVar"), .childrenCount = 1}},
                    {.type   = JSON_NODE_TYPE_STRING,
                     .string = {.value = CHAR_SLICE("escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\"), .childrenCount = 0}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"strVar\": \"\"}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("strVar"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE(""), .childrenCount = 0}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"a\":\"\\uAbcD\"}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("a"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("\\uAbcD"), .childrenCount = 0}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"a\":\"str\\u0000\"}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("a"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("str\\u0000"), .childrenCount = 0}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"a\":\"\\uFFFFstr\"}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("a"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("\\uFFFFstr"), .childrenCount = 0}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"a\":[\"\\u0280\"]}"),
            .poolSize      = 4,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 4,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("a"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_ARRAY, .other = {.childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("\\u0280"), .childrenCount = 0}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"a\":\"str\\uFFGFstr\"}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_INVALID,
            .wantNodeCount = 0,
            .rules         = {0},
        },
        {
            .src           = CHAR_SLICE("{\"a\":\"str\\u@FfF\"}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_INVALID,
            .wantNodeCount = 0,
            .rules         = {0},
        },
        {
            .src           = CHAR_SLICE("{{\"a\":[\"\\u028\"]}"),
            .poolSize      = 4,
            .wantErr       = JSON_PARSE_ERROR_INVALID,
            .wantNodeCount = 0,
            .rules         = {0},
        },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(ParseTest); i++) {
        const auto tt = tests[i];
        TEST_MESSAGE(tt.src.arr);
        ParseTest_Run(tt);
    }
}

// void Test_PartialString(void) {
//     const auto src = CHAR_SLICE("{\"x\": \"va\\\\ue\", \"y\": \"value y\"}");
//     JsonNode   arr[5];
//     JsonNodes  dst = {
//          .arr = arr,
//          .cap = sizeof(arr) / sizeof(JsonNode),
//     };
//
//     for (size_t i = 1; i <= src.len; i++) {
//         const auto r = JsonNodes_Parse(&dst, CharSlice_View(src, 0, i));
//         if (i != src.len) {
//             TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_PARTIAL, r.err);
//             continue;
//         }
//
//         TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, r.err);
//         TEST_ASSERT_EQUAL(5, dst.len);
//         tokeq(
//             &dst, src, JSON_NODE_TYPE_OBJECT, -1, -1, 2, JSON_NODE_TYPE_STRING, "x", 1, JSON_NODE_TYPE_STRING,
//             "va\\\\ue", 0, JSON_NODE_TYPE_STRING, "y", 1, JSON_NODE_TYPE_STRING, "value y", 0
//         );
//     }
// }
//
// void Test_PartialArray(void) {
//     const auto src = CHAR_SLICE("[ 1, true, [123, \"hello\"]]");
//     JsonNode   arr[10];
//     JsonNodes  dst = {
//          .arr = arr,
//          .cap = sizeof(arr) / sizeof(JsonNode),
//     };
//
//     for (size_t i = 1; i <= src.len; i++) {
//         const auto r = JsonNodes_Parse(&dst, CharSlice_View(src, 0, i));
//         if (i != src.len) {
//             TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_PARTIAL, r.err);
//             continue;
//         }
//
//         TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, r.err);
//         TEST_ASSERT_EQUAL(6, dst.len);
//         tokeq(
//             &dst, src, JSON_NODE_TYPE_ARRAY, -1, -1, 3, JSON_NODE_TYPE_PRIMITIVE, "1", JSON_NODE_TYPE_PRIMITIVE,
//             "true", JSON_NODE_TYPE_ARRAY, -1, -1, 2, JSON_NODE_TYPE_PRIMITIVE, "123", JSON_NODE_TYPE_STRING, "hello",
//             0
//         );
//     }
// }

void Test_ArrayNodesExhausted(void) {
    for (int i = 0; i < 6; i++) {
        constexpr size_t arrLen      = 10;
        JsonNode         arr[arrLen] = {};
        JsonNodes        dst         = {.arr = arr, .cap = i};
        const auto       src         = CHAR_SLICE("  [ 1, true, [123, \"hello\"]]");
        const auto       r1          = JsonNodes_Parse(&dst, src);
        TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED, r1.err);
    }
}

void Test_UnquotedKeys(void) {
    JsonNode   arr[10];
    JsonNodes  dst = {.arr = arr, .cap = sizeof(arr) / sizeof(JsonNode)};
    const auto src = CHAR_SLICE("key1: \"value\"\nkey2 : 123");
    const auto r   = JsonNodes_Parse(&dst, src);
    TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_INVALID, r.err);
}

void Test_Issue22(void) {
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

void Test_Issue27(void) {
    const ParseTest test = {
        .src           = CHAR_SLICE("{ \"name\" : \"Jack\", \"age\" : 27 } { \"name\" : \"Anna\", "),
        .poolSize      = 8,
        .wantErr       = JSON_PARSE_ERROR_OK,
        .wantNodeCount = 5,
        .rules         = {
            {
                        .type  = JSON_NODE_TYPE_OBJECT,
                        .other = {.offset = 0, .len = 31, .childrenCount = 2},
            },
            {
                        .type   = JSON_NODE_TYPE_STRING,
                        .string = {.value = CHAR_SLICE("name"), .childrenCount = 1},
            },
            {
                        .type   = JSON_NODE_TYPE_STRING,
                        .string = {.value = CHAR_SLICE("Jack"), .childrenCount = 0},
            },
            {
                        .type   = JSON_NODE_TYPE_STRING,
                        .string = {.value = CHAR_SLICE("age"), .childrenCount = 1},
            },
            {
                        .type      = JSON_NODE_TYPE_PRIMITIVE,
                        .primitive = {.value = CHAR_SLICE("27")},
            },
        },
    };

    ParseTest_Run(test);
}

void Test_InputLength(void) {
    JsonNode  arr[10] = {};
    JsonNodes dst     = {
            .arr = arr,
            .cap = sizeof(arr) / sizeof(JsonNode),
    };
    const auto              src   = CHAR_SLICE("{\"a\": 0}garbage");
    const NodeMatchingRules rules = {
        {.type = JSON_NODE_TYPE_OBJECT, .other = {.childrenCount = 1}},
        {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("a"), .childrenCount = 1}},
        {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("0")}},

    };
    const auto r = JsonNodes_Parse(&dst, src);
    TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, r.err);
    TEST_ASSERT_EQUAL(8, r.offset);
    TEST_ASSERT_EQUAL(3, dst.len);
    NodeMatchingRules_AssertMatches(rules, src, dst);
}

void Test_Count(void) {
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
        const auto tt      = tests[i];
        JsonNode   arr[10] = {};
        JsonNodes  dst     = {.arr = arr, .cap = sizeof(arr) / sizeof(JsonNode)};
        const Str  src     = {.arr = tt.src, .len = strlen(tt.src)};

        TEST_MESSAGE(tt.name);
        const auto r = JsonNodes_Parse(&dst, src);
        TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, r.err);
        TEST_ASSERT_EQUAL(tt.wantLen, dst.len);
    }
}

void Test_NonStrict(void) {
    const ParseTest tests[] = {
        {
            .src      = CHAR_SLICE("a: 0garbage"),
            .poolSize = 2,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("Day : 26\nMonth : Sep\n\nYear: 12"),
            .poolSize = 6,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("\"key {1\": 1234"),
            .poolSize = 2,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(ParseTest); i++) {
        const auto tt = tests[i];
        TEST_MESSAGE(tt.src.arr);
        ParseTest_Run(tt);
    }
}

void Test_UnmatchedBrackets(void) {
    // C
    const ParseTest tests[] = {
        {
            .src      = CHAR_SLICE("\"key 1\": 1234}"),
            .poolSize = 2,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("{\"key 1\": 1234"),
            .poolSize = 3,
            .wantErr  = JSON_PARSE_ERROR_PARTIAL,
        },
        {
            .src           = CHAR_SLICE("{\"key 1\": 1234}}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.offset = 0, .len = 15, .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("key 1"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("1234")}},
                },
        },
        {
            .src      = CHAR_SLICE("\"key 1\"}: 1234"),
            .poolSize = 3,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src           = CHAR_SLICE("{\"key {1\": 1234}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.offset = 0, .len = 16, .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("key {1"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("1234")}},
                },
        },
        {
            .src           = CHAR_SLICE("{\"key 1\":{\"key 2\": 1234}"),
            .poolSize      = 5,
            .wantErr       = JSON_PARSE_ERROR_PARTIAL,
            .wantNodeCount = 5,
        },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(ParseTest); i++) {
        const auto tt = tests[i];
        TEST_MESSAGE(tt.src.arr);
        ParseTest_Run(tt);
    }
}

void Test_ObjectKey(void) {
    // C
    const ParseTest tests[] = {
        {
            .src           = CHAR_SLICE("{\"key\": 1}"),
            .poolSize      = 3,
            .wantErr       = JSON_PARSE_ERROR_OK,
            .wantNodeCount = 3,
            .rules =
                {
                    {.type = JSON_NODE_TYPE_OBJECT, .other = {.offset = 0, .len = 10, .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_STRING, .string = {.value = CHAR_SLICE("key"), .childrenCount = 1}},
                    {.type = JSON_NODE_TYPE_PRIMITIVE, .primitive = {.value = CHAR_SLICE("1")}},
                },
        },
        {
            .src      = CHAR_SLICE("{true: 1}"),
            .poolSize = 3,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("{1: 1}"),
            .poolSize = 3,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("{{\"key\": 1}: 2}"),
            .poolSize = 5,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
        {
            .src      = CHAR_SLICE("{[1,2]: 2}"),
            .poolSize = 5,
            .wantErr  = JSON_PARSE_ERROR_INVALID,
        },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(ParseTest); i++) {
        const auto tt = tests[i];
        TEST_MESSAGE(tt.src.arr);
        ParseTest_Run(tt);
    }
}

void Example_JsonParse() {
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

        // printf("%s", msg);
    }

    for (size_t i = 0; i < sizeof(nodes) / sizeof(nodes[0]); i++) {
        const auto n = nodes[i];
        if (n.type == JSON_NODE_TYPE_UNSPECIFIED) {
            break;
        }

        char *typeStr;
        switch (n.type) {
            case JSON_NODE_TYPE_UNSPECIFIED:
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

        const auto content = Str_View(src, n.offset, n.offset + n.len);
        printf(
            "%-10s| %4zd, %4zd, %4zd| \"%.*s\"\n", typeStr, n.offset, n.len, n.childrenCount, (int)content.len,
            content.arr
        );
    }
}

void Test_JsonWrite() {
    auto      dst = CharSlice_OnStack(0, 1024);
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
    CharSlice_WriteJsonStr(&dst, &s, CHAR_SLICE("Foo"));
    CharSlice_WriteJsonEnd(&dst, &s);

    CharSlice_WriteJsonKey(&dst, &s, CHAR_SLICE("arrField"));
    CharSlice_WriteJsonStart(&dst, &s, '[');
    CharSlice_WriteJsonBool(&dst, &s, true);
    CharSlice_WriteJsonNumeric(&dst, &s, CHAR_SLICE("456"));
    CharSlice_WriteJsonStr(&dst, &s, CHAR_SLICE("Bar"));
    CharSlice_WriteJsonEnd(&dst, &s);

    CharSlice_WriteJsonEnd(&dst, &s);

    TEST_ASSERT_EQUAL_STRING(
        "{\"objField\":{\"numericField\":123,\"strField\":\"Foo\"},\"arrField\":[true,456,\"Bar\"]}", dst.arr
    );
}

int main(void) {
    mem = Arena_Wrap(buffer);

    UNITY_BEGIN();

    RUN_TEST(Test_Empty);
    RUN_TEST(Test_Object);
    RUN_TEST(test_array);
    RUN_TEST(Test_Primitive);
    RUN_TEST(Test_String);
    // RUN_TEST(Test_PartialString);
    // RUN_TEST(Test_PartialArray);
    RUN_TEST(Test_ArrayNodesExhausted);
    RUN_TEST(Test_UnquotedKeys);
    RUN_TEST(Test_Issue22);
    RUN_TEST(Test_Issue27);
    RUN_TEST(Test_InputLength);
    RUN_TEST(Test_Count);
    RUN_TEST(Test_NonStrict);
    RUN_TEST(Test_UnmatchedBrackets);
    RUN_TEST(Test_ObjectKey);

    RUN_TEST(Example_JsonParse);
    RUN_TEST(Test_JsonWrite);

    printf("mem: %zd/%zd\n", mem.offset, mem.cap);
    return UNITY_END();
}
