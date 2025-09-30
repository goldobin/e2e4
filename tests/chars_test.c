#include "../include/chars.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "../include/arena.h"

auto mem = Arena_OnStack(1024);

void setUp() {}
void tearDown() {}

void Test_Str_At() {
    const auto s = STR("hello");

    // Test valid indices
    TEST_ASSERT_EQUAL_CHAR('h', Str_At(s, 0));
    TEST_ASSERT_EQUAL_CHAR('e', Str_At(s, 1));
    TEST_ASSERT_EQUAL_CHAR('l', Str_At(s, 2));
    TEST_ASSERT_EQUAL_CHAR('l', Str_At(s, 3));
    TEST_ASSERT_EQUAL_CHAR('o', Str_At(s, 4));

    // Test boundary
    TEST_ASSERT_EQUAL_CHAR('\0', s.arr[5]);
}

void Test_CharBuff_WriteOne() {
    CharBuff b = CharBuff_OnStack(0, 10);
    CharBuff_WriteStr(&b, STR("hello"));

    // Update valid indices
    CharBuff_WriteCharAt(&b, 0, 'H');
    TEST_ASSERT_EQUAL_CHAR('H', CharBuff_At(b, 0));

    CharBuff_WriteCharAt(&b, 4, 'O');
    TEST_ASSERT_EQUAL_CHAR('O', CharBuff_At(b, 4));

    // Verify other characters remain unchanged
    TEST_ASSERT_EQUAL_CHAR('e', CharBuff_At(b, 1));
    TEST_ASSERT_EQUAL_CHAR('l', CharBuff_At(b, 2));
    TEST_ASSERT_EQUAL_CHAR('l', CharBuff_At(b, 3));
}

void Test_CharBuff_WriteChar() {
    CharBuff b = CharBuff_OnStack(0, 10);
    CharBuff_WriteStr(&b, STR("hell"));

    // Test successful append
    const auto written01 = CharBuff_WriteChar(&b, 'o');
    TEST_ASSERT_EQUAL(1, written01);
    TEST_ASSERT_EQUAL(5, b.len);
    TEST_ASSERT_EQUAL_CHAR('o', CharBuff_At(b, 4));

    // Test appending when at capacity
    auto       fullBuff  = CharBuff_OnStack(5, 5);
    const auto written02 = CharBuff_WriteChar(&fullBuff, 'x');
    TEST_ASSERT_EQUAL(0, written02);
    TEST_ASSERT_EQUAL(5, fullBuff.len);
}

void Test_CharBuff_Write(void) {
    CharBuff b01 = CharBuff_OnStack(0, 20);
    CharBuff_WriteStr(&b01, STR("hello"));
    const auto written01 = CharBuff_WriteStr(&b01, STR(" world"));
    TEST_ASSERT_EQUAL_size_t(6, written01);
    TEST_ASSERT_EQUAL(11, b01.len);
    TEST_ASSERT_EQUAL_STRING("hello world", b01.arr);

    CharBuff b02 = CharBuff_OnStack(0, 5);
    CharBuff_WriteStr(&b02, STR("hi"));
    const auto appended02 = CharBuff_WriteStr(&b02, STR(" there!"));
    TEST_ASSERT_EQUAL(3, appended02);
    TEST_ASSERT_EQUAL(5, b02.len);
    TEST_ASSERT_EQUAL_STRING_LEN("hi th", b02.arr, 5);
}

void Test_Str_View() {
    const auto s = STR("hello world");

    // Test normal view
    const auto view1 = Str_View(s, 0, 5);
    TEST_ASSERT_EQUAL_PTR(s.arr, view1.arr);
    TEST_ASSERT_EQUAL(5, view1.len);

    // Test substring view
    const auto view2 = Str_View(s, 6, 11);
    TEST_ASSERT_EQUAL_PTR(s.arr + 6, view2.arr);
    TEST_ASSERT_EQUAL(5, view2.len);

    // Test single character view
    const auto view3 = Str_View(s, 0, 1);
    TEST_ASSERT_EQUAL_PTR(s.arr, view3.arr);
    TEST_ASSERT_EQUAL(1, view3.len);

    // Test empty view
    const auto view4 = Str_View(s, 5, 5);
    TEST_ASSERT_TRUE(view4.arr == nullptr);
    TEST_ASSERT_EQUAL(0, view4.len);
}

void Test_CharBuff_EdgeCases(void) {
    CharBuff empty = CharBuff_OnStack(0, 10);

    const auto written = CharBuff_WriteChar(&empty, 'a');
    TEST_ASSERT_EQUAL(1, written);
    TEST_ASSERT_EQUAL(1, empty.len);
    TEST_ASSERT_EQUAL_CHAR('a', CharBuff_At(empty, 0));

    CharBuff b = CharBuff_OnStack(0, 5);
    CharBuff_WriteChar(&b, 'x');
    const auto view = CharBuff_View(b, 0, 1);
    TEST_ASSERT_EQUAL(1, view.len);
    TEST_ASSERT_EQUAL_CHAR('x', view.arr[0]);
}

void Test_CharBuff_WriteDiff() {
    typedef struct {
        const char*  name;
        const Str    s1;
        const Str    s2;
        const size_t dstCap;
        const char*  wantDiff;
    } test;

    const test tests[] = {
        {
            .name     = "case 0.1",
            .s1       = STR("some str"),
            .s2       = STR("some str"),
            .dstCap   = 10,
            .wantDiff = "",
        },
        {
            .name     = "case 1.1",
            .s1       = STR("some str1 test"),
            .s2       = STR("some str"),
            .dstCap   = 32,
            .wantDiff = "some str{1 test|\\0}",
        },
        {
            .name     = "case 1.2",
            .s1       = STR("some str"),
            .s2       = STR("some str2 test"),
            .dstCap   = 32,
            .wantDiff = "some str{\\0|2 test}",
        },
        {
            .name     = "case 2.1",
            .s1       = STR("some str1 foo"),
            .s2       = STR("some str2 bar"),
            .dstCap   = 32,
            .wantDiff = "some str{1 foo|2 bar}",
        },
        {
            .name     = "case 2.2",
            .s1       = STR("some str1 foo foo foo foo foo"),
            .s2       = STR("some str2 bar"),
            .dstCap   = 32,
            .wantDiff = "some str{1 f...|2 bar}",
        }
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(test); i++) {
        const test tt = tests[i];
        TEST_MESSAGE(tt.name);
        CharBuff dst = {};
        if (!CharBuff_Alloc(&dst, tt.dstCap, &mem)) {
            TEST_FAIL_MESSAGE("not enough memory in arena");
        }
        CharBuff_WriteDiff(&dst, tt.s1, tt.s2);

        TEST_ASSERT_EQUAL_STRING(tt.wantDiff, dst.arr);
    }
}

void Test_CharBuff_WriteF() {
    CharBuff dst1 = CharBuff_OnStack(0, 64);
    CharBuff dst2 = CharBuff_OnStack(0, 8);

    const auto printed1 = CharBuff_WriteF(&dst1, "test %d, %d, %d", 1, 2, 3);
    const auto printed2 = CharBuff_WriteF(&dst2, "test %d, %d, %d", 1, 2, 3);

    TEST_ASSERT_EQUAL_STRING("test 1, 2, 3", dst1.arr);
    TEST_ASSERT_EQUAL(12, printed1);
    TEST_ASSERT_EQUAL_STRING("test 1,", dst2.arr);
    TEST_ASSERT_EQUAL(7, printed2);
}

void Test_CharBuff_ReadWriteFile(void) {
    char  fileBuf[1024] = {};
    auto  in            = CharBuff_OnStack(0, 64);
    auto  out           = CharBuff_OnStack(0, 64);
    FILE* f             = fmemopen(fileBuf, sizeof(fileBuf), "w+");

    CharBuff_WriteStr(&in, STR("arbitrary text"));
    File_WriteCharBuff(f, in);
    fseek(f, 0, SEEK_SET);
    CharBuff_WriteFile(&out, f);

    TEST_ASSERT_TRUE(CharBuff_Equals(in, out));
}

void Test_Str_IndexOf() {
    typedef struct {
        const char* name;
        const Str   src;
        const Str   v;
        const int   want;
    } test;

    const test tests[] = {
        {
            .name = "case 0.1",
            .src  = STR(""),
            .v    = STR(""),
            .want = -1,
        },
        {
            .name = "case 0.2",
            .src  = STR("test 01"),
            .v    = STR(""),
            .want = -1,
        },
        {
            .name = "case 0.3",
            .src  = STR(""),
            .v    = STR("str"),
            .want = -1,
        },
        {
            .name = "case 0.4",
            .src  = STR("some str"),
            .v    = STR("none"),
            .want = -1,
        },
        {
            .name = "case 0.5",
            .src  = STR("some str"),
            .v    = STR("rx"),
            .want = -1,
        },
        {
            .name = "case 0.6",
            .src  = STR("some str"),
            .v    = STR("r\0"),
            .want = -1,
        },
        {
            .name = "case 1.1.1 one char",
            .src  = STR("some str"),
            .v    = STR("s"),
            .want = 0,
        },
        {
            .name = "case 1.1.2 one char",
            .src  = STR("some str"),
            .v    = STR("o"),
            .want = 1,
        },
        {
            .name = "case 1.1.3 one char",
            .src  = STR("some str"),
            .v    = STR(" "),
            .want = 4,
        },
        {
            .name = "case 1.1.4 one char",
            .src  = STR("some str"),
            .v    = STR("r"),
            .want = 7,
        },
        {
            .name = "case 1.2 substr",
            .src  = STR("some str"),
            .v    = STR("some"),
            .want = 0,
        },
        {
            .name = "case 1.3 substr",
            .src  = STR("some str"),
            .v    = STR("str"),
            .want = 5,
        },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(test); i++) {
        const test tt = tests[i];

        //        if (strcmp(tt.name, "case 1.1.4 one char") != 0) {
        //            continue;
        //        }

        TEST_MESSAGE(tt.name);
        const auto got = Str_IndexOf(tt.src, tt.v);
        TEST_ASSERT_EQUAL(tt.want, got);
    }
}

void Test_Str_Trim() {
    typedef struct {
        const char* name;
        const Str   src;
        const Str   cutset;
        const Str   want;
    } test;

    const test tests[] = {
        {
            .name   = "case 0.1",
            .src    = STR(""),
            .cutset = STR(" "),
            .want   = STR(""),
        },
        {
            .name   = "case 0.2",
            .src    = STR("test1"),
            .cutset = STR(" "),
            .want   = STR("test1"),
        },
        {
            .name   = "case 0.3",
            .src    = STR("test2"),
            .cutset = STR("!"),
            .want   = STR("test2"),
        },
        {
            .name   = "case 1.1 spaces",
            .src    = STR("  test3  "),
            .cutset = STR(" "),
            .want   = STR("test3"),
        },
        {
            .name   = "case 1.2 cutset",
            .src    = STR("!!!test4---"),
            .cutset = STR("!-"),
            .want   = STR("test4"),
        },
        {
            .name   = "case 1.3 special chars",
            .src    = STR("\t\r\n test5   \r\n"),
            .cutset = STR(" \t\n\r"),
            .want   = STR("test5"),
        }
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(test); i++) {
        const test tt = tests[i];

        //        if (strcmp(tt.name, "case 1.1.4 one char") != 0) {
        //            continue;
        //        }

        TEST_MESSAGE(tt.name);
        const auto got = Str_Trim(tt.src, tt.cutset);
        TEST_ASSERT_TRUE(Str_Equals(tt.want, got));
    }
}

void Test_Strings_Split() {
    typedef struct {
        const char*   name;
        const Str     src;
        const Str     sep;
        const size_t  dstCap;
        const Strings want;
        const size_t  wantOffset;
    } test;

    const test tests[] = {
        {
            .name   = "case 0.1 empty src and sep",
            .src    = STR(""),
            .sep    = STR(""),
            .dstCap = 1,
        },
        {
            .name       = "case 0.2 empty sep",
            .src        = STR("test"),
            .sep        = STR(""),
            .dstCap     = 10,
            .want       = Strings_Of(STR("t"), STR("e"), STR("s"), STR("t")),
            .wantOffset = 4,
        },
        {
            .name       = "case 0.3 empty src",
            .src        = STR(""),
            .sep        = STR(" "),
            .dstCap     = 10,
            .want       = Strings_Of(STR("")),
            .wantOffset = 4,
        },
        {
            .name       = "case 0.4 not matching separator",
            .src        = STR("test01"),
            .sep        = STR(" "),
            .dstCap     = 1,
            .want       = Strings_Of(STR("test01")),
            .wantOffset = 6,
        },
        {
            .name       = "case 0.5 partially matching separator",
            .src        = STR("test02"),
            .sep        = STR("tx"),
            .dstCap     = 1,
            .want       = Strings_Of(STR("test02")),
            .wantOffset = 6,
        },
        {
            .name       = "case 1.1.1 one char split",
            .src        = STR("test01"),
            .sep        = STR("t"),
            .dstCap     = 10,
            .want       = Strings_Of(STR(""), STR("es"), STR("01")),
            .wantOffset = 6,
        },
        {
            .name       = "case 1.1.2 one char split",
            .src        = STR("test01 test02"),
            .sep        = STR(" "),
            .dstCap     = 10,
            .want       = Strings_Of(STR("test01"), STR("test02")),
            .wantOffset = 9,
        },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(test); i++) {
        const test tt = tests[i];

        //        if (strcmp(tt.name, "case 1.1.1 one char split") != 0) {
        //            continue;
        //        }

        TEST_MESSAGE(tt.name);
        Strings got = {};
        if (!Strings_Alloc(&got, tt.dstCap, &mem)) {
            TEST_FAIL_MESSAGE("not enough memory in arena");
        };
        Strings_Split(&got, tt.src, tt.sep);
        TEST_ASSERT_TRUE(Strings_Equals(tt.want, got));
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(Test_Str_At);
    RUN_TEST(Test_Str_View);
    RUN_TEST(Test_Str_IndexOf);
    RUN_TEST(Test_Str_Trim);
    RUN_TEST(Test_CharBuff_WriteOne);
    RUN_TEST(Test_CharBuff_WriteChar);
    RUN_TEST(Test_CharBuff_Write);
    RUN_TEST(Test_CharBuff_EdgeCases);
    RUN_TEST(Test_CharBuff_WriteDiff);
    RUN_TEST(Test_CharBuff_WriteF);
    RUN_TEST(Test_CharBuff_ReadWriteFile);
    RUN_TEST(Test_Strings_Split);

    printf("mem: %zd/%zd\n", mem.offset, mem.cap);

    return UNITY_END();
}
