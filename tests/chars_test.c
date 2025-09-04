#include "../include/chars.h"

#include <assert.h>
#include <stdlib.h>
#include <unity.h>

#include "../include/arena.h"

uint8_t buffer[512] = {};
Arena   mem         = {};

void setUp(void) {}

void tearDown(void) {}

void Test_CharSlice_At(void) {
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

CharBuff createSlice() { return CharBuff_OnStack(0, 10); }

void Test_CharSlice_WriteOne(void) {
    CharBuff s = CharBuff_OnStack(0, 10);
    CharBuff_WriteStr(&s, STR("hello"));

    // Update valid indices
    CharBuff_WriteCharAt(&s, 0, 'H');
    TEST_ASSERT_EQUAL_CHAR('H', CharBuff_At(s, 0));

    CharBuff_WriteCharAt(&s, 4, 'O');
    TEST_ASSERT_EQUAL_CHAR('O', CharBuff_At(s, 4));

    // Verify other characters remain unchanged
    TEST_ASSERT_EQUAL_CHAR('e', CharBuff_At(s, 1));
    TEST_ASSERT_EQUAL_CHAR('l', CharBuff_At(s, 2));
    TEST_ASSERT_EQUAL_CHAR('l', CharBuff_At(s, 3));

    // CharBuff s2 = STR("test0123456789");
    // CharBuff_WriteCharAt(&s2, 0, 'v');
    //
    // printf("%s\n", s2.arr);

    // CharBuff s3 = createSlice();
    // CharBuff_WriteChar(&s3, 'v');
}

void Test_CharSlice_WriteOneBack(void) {
    CharBuff s = CharBuff_OnStack(0, 10);
    CharBuff_WriteStr(&s, STR("hell"));

    // Test successful append
    const auto appended01 = CharBuff_WriteChar(&s, 'o');
    TEST_ASSERT_EQUAL(1, appended01);
    TEST_ASSERT_EQUAL(5, s.len);
    TEST_ASSERT_EQUAL_CHAR('o', CharBuff_At(s, 4));

    // Test appending when at capacity
    auto       fullSlice  = CharBuff_OnStack(5, 5);
    const auto appended02 = CharBuff_WriteChar(&fullSlice, 'x');
    TEST_ASSERT_EQUAL(0, appended02);
    TEST_ASSERT_EQUAL(5, fullSlice.len);
}

void Test_CharSlice_WriteBack(void) {
    CharBuff   slice01 = CharBuff_OnStack(0, 20);
    const auto slice02 = STR(" world");

    CharBuff_WriteStr(&slice01, STR("hello"));

    // Test successful append
    const auto writen01 = CharBuff_WriteStr(&slice01, slice02);
    TEST_ASSERT_EQUAL_size_t(6, writen01);
    TEST_ASSERT_EQUAL(11, slice01.len);

    TEST_ASSERT_EQUAL_STRING("hello world", slice01.arr);

    // Test append when it would exceed capacity
    CharBuff smallSlice = CharBuff_OnStack(0, 5);
    CharBuff_WriteStr(&smallSlice, STR("hi"));
    const auto largeSlice = STR(" there!");
    const auto appended02 = CharBuff_WriteStr(&smallSlice, largeSlice);
    TEST_ASSERT_EQUAL(3, appended02);
    TEST_ASSERT_EQUAL(5, smallSlice.len);
    TEST_ASSERT_EQUAL_STRING_LEN("hi th", smallSlice.arr, 5);
}

void Test_CharSlice_View(void) {
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
    TEST_ASSERT_EQUAL_PTR(s.arr + 5, view4.arr);
    TEST_ASSERT_EQUAL(0, view4.len);
}

void Test_CharSlice_EdgeCases(void) {
    // Test empty slice
    CharBuff empty = CharBuff_OnStack(0, 10);

    const auto writen = CharBuff_WriteChar(&empty, 'a');
    TEST_ASSERT_EQUAL(1, writen);
    TEST_ASSERT_EQUAL(1, empty.len);
    TEST_ASSERT_EQUAL_CHAR('a', CharBuff_At(empty, 0));

    // Test a single character slice
    CharBuff single = CharBuff_OnStack(0, 5);
    CharBuff_WriteChar(&single, 'x');
    const auto view = CharBuff_View(single, 0, 1);
    TEST_ASSERT_EQUAL(1, view.len);
    TEST_ASSERT_EQUAL_CHAR('x', view.arr[0]);
}

void Test_CharSlice_WriteDiff(void) {
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
            TEST_MESSAGE("not enough memory in arena");
            TEST_FAIL();
        }
        CharBuff_WriteDiff(&dst, tt.s1, tt.s2);

        TEST_ASSERT_EQUAL_STRING(tt.wantDiff, dst.arr);
    }
}

void Test_CharSlice_WriteF(void) {
    CharBuff dst1 = CharBuff_OnStack(0, 64);
    CharBuff dst2 = CharBuff_OnStack(0, 8);

    const auto printed1 = CharBuff_WriteF(&dst1, "test %d, %d, %d", 1, 2, 3);
    const auto printed2 = CharBuff_WriteF(&dst2, "test %d, %d, %d", 1, 2, 3);

    TEST_ASSERT_EQUAL_STRING("test 1, 2, 3", dst1.arr);
    TEST_ASSERT_EQUAL(12, printed1);
    TEST_ASSERT_EQUAL_STRING("test 1,", dst2.arr);
    TEST_ASSERT_EQUAL(7, printed2);
}

void Test_CharSlice_ReadWriteFile(void) {
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

int main(void) {
    mem = Arena_Wrap(buffer);

    UNITY_BEGIN();
    RUN_TEST(Test_CharSlice_At);
    RUN_TEST(Test_CharSlice_WriteOne);
    RUN_TEST(Test_CharSlice_WriteOneBack);
    RUN_TEST(Test_CharSlice_WriteBack);
    RUN_TEST(Test_CharSlice_View);
    RUN_TEST(Test_CharSlice_EdgeCases);
    RUN_TEST(Test_CharSlice_WriteDiff);
    RUN_TEST(Test_CharSlice_WriteF);
    RUN_TEST(Test_CharSlice_ReadWriteFile);

    printf("mem: %zd/%zd\n", mem.offset, mem.cap);

    return UNITY_END();
}
