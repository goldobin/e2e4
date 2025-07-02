#include "../include/char_slice.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "../include/arena.h"

uint8_t buffer[512] = {};
Arena   mem         = {};

void setUp(void) {}

void tearDown(void) {}

void Test_CharSlice_At(void) {
    const auto slice = CHAR_SLICE("hello");

    // Test valid indices
    TEST_ASSERT_EQUAL_CHAR('h', CharSlice_At(&slice, 0));
    TEST_ASSERT_EQUAL_CHAR('e', CharSlice_At(&slice, 1));
    TEST_ASSERT_EQUAL_CHAR('l', CharSlice_At(&slice, 2));
    TEST_ASSERT_EQUAL_CHAR('l', CharSlice_At(&slice, 3));
    TEST_ASSERT_EQUAL_CHAR('o', CharSlice_At(&slice, 4));

    // Test boundary
    TEST_ASSERT_EQUAL_CHAR('\0', slice.arr[5]);
}

void Test_CharSlice_WriteOne(void) {
    CharSlice slice = CharSlice_Make(0, 10);
    CharSlice_Write(&slice, CHAR_SLICE("hello"));

    // Update valid indices
    CharSlice_WriteCharAt(&slice, 0, 'H');
    TEST_ASSERT_EQUAL_CHAR('H', CharSlice_At(&slice, 0));

    CharSlice_WriteCharAt(&slice, 4, 'O');
    TEST_ASSERT_EQUAL_CHAR('O', CharSlice_At(&slice, 4));

    // Verify other characters remain unchanged
    TEST_ASSERT_EQUAL_CHAR('e', CharSlice_At(&slice, 1));
    TEST_ASSERT_EQUAL_CHAR('l', CharSlice_At(&slice, 2));
    TEST_ASSERT_EQUAL_CHAR('l', CharSlice_At(&slice, 3));
}

void Test_CharSlice_WriteOneBack(void) {
    CharSlice slice = CharSlice_Make(0, 10);
    CharSlice_Write(&slice, CHAR_SLICE("hell"));

    // Test successful append
    const auto appended01 = CharSlice_WriteChar(&slice, 'o');
    TEST_ASSERT_EQUAL(1, appended01);
    TEST_ASSERT_EQUAL(5, slice.len);
    TEST_ASSERT_EQUAL_CHAR('o', CharSlice_At(&slice, 4));

    // Test appending when at capacity
    CharSlice  fullSlice  = CHAR_SLICE("12345");
    const auto appended02 = CharSlice_WriteChar(&fullSlice, 'x');
    TEST_ASSERT_EQUAL(0, appended02);
    TEST_ASSERT_EQUAL(5, fullSlice.len);
}

void Test_CharSlice_WriteBack(void) {
    CharSlice       slice01 = CharSlice_Make(0, 20);
    const CharSlice slice02 = CHAR_SLICE(" world");

    CharSlice_Write(&slice01, CHAR_SLICE("hello"));

    // Test successful append
    const auto writen01 = CharSlice_Write(&slice01, slice02);
    TEST_ASSERT_EQUAL_size_t(6, writen01);
    TEST_ASSERT_EQUAL(11, slice01.len);

    TEST_ASSERT_EQUAL_STRING("hello world", slice01.arr);

    // Test append when it would exceed capacity
    CharSlice smallSlice = CharSlice_Make(0, 5);
    CharSlice_Write(&smallSlice, CHAR_SLICE("hi"));
    const CharSlice largeSlice = CHAR_SLICE(" there!");
    const auto      appended02 = CharSlice_Write(&smallSlice, largeSlice);
    TEST_ASSERT_EQUAL(3, appended02);
    TEST_ASSERT_EQUAL(5, smallSlice.len);
    TEST_ASSERT_EQUAL_STRING_LEN("hi th", smallSlice.arr, 5);
}

void Test_CharSlice_View(void) {
    const CharSlice slice = CHAR_SLICE("hello world");

    // Test normal view
    const CharSlice view1 = CharSlice_View(&slice, 0, 5);
    TEST_ASSERT_EQUAL_PTR(slice.arr, view1.arr);
    TEST_ASSERT_EQUAL(5, view1.len);
    TEST_ASSERT_EQUAL(5, view1.cap);

    // Test substring view
    const CharSlice view2 = CharSlice_View(&slice, 6, 11);
    TEST_ASSERT_EQUAL_PTR(slice.arr + 6, view2.arr);
    TEST_ASSERT_EQUAL(5, view2.len);
    TEST_ASSERT_EQUAL(5, view2.cap);

    // Test single character view
    const CharSlice view3 = CharSlice_View(&slice, 0, 1);
    TEST_ASSERT_EQUAL_PTR(slice.arr, view3.arr);
    TEST_ASSERT_EQUAL(1, view3.len);
    TEST_ASSERT_EQUAL(1, view3.cap);

    // Test empty view
    const CharSlice view4 = CharSlice_View(&slice, 5, 5);
    TEST_ASSERT_EQUAL_PTR(slice.arr + 5, view4.arr);
    TEST_ASSERT_EQUAL(0, view4.len);
    TEST_ASSERT_EQUAL(0, view4.cap);
}

void Test_CharSlice_EdgeCases(void) {
    // Test empty slice
    CharSlice empty = CharSlice_Make(0, 10);

    const auto writen = CharSlice_WriteChar(&empty, 'a');
    TEST_ASSERT_EQUAL(1, writen);
    TEST_ASSERT_EQUAL(1, empty.len);
    TEST_ASSERT_EQUAL_CHAR('a', CharSlice_At(&empty, 0));

    // Test a single character slice
    CharSlice single = CharSlice_Make(0, 5);
    CharSlice_WriteChar(&single, 'x');
    const CharSlice view = CharSlice_View(&single, 0, 1);
    TEST_ASSERT_EQUAL(1, view.len);
    TEST_ASSERT_EQUAL_CHAR('x', view.arr[0]);
}

void Test_CharSlice_Diff(void) {
    typedef struct {
        const char*     name;
        const CharSlice s1;
        const CharSlice s2;
        const size_t    dstCap;
        const char*     wantDiff;
    } test;

    const test tests[] = {
        {
            .name     = "case 0.1",
            .s1       = CHAR_SLICE("some str"),
            .s2       = CHAR_SLICE("some str"),
            .dstCap   = 10,
            .wantDiff = "",
        },
        {
            .name     = "case 1.1",
            .s1       = CHAR_SLICE("some str1 test"),
            .s2       = CHAR_SLICE("some str"),
            .dstCap   = 32,
            .wantDiff = "some str{1 test|\\0}",
        },
        {
            .name     = "case 1.2",
            .s1       = CHAR_SLICE("some str"),
            .s2       = CHAR_SLICE("some str2 test"),
            .dstCap   = 32,
            .wantDiff = "some str{\\0|2 test}",
        },
        {
            .name     = "case 2.1",
            .s1       = CHAR_SLICE("some str1 foo"),
            .s2       = CHAR_SLICE("some str2 bar"),
            .dstCap   = 32,
            .wantDiff = "some str{1 foo|2 bar}",
        },
        {
            .name     = "case 2.2",
            .s1       = CHAR_SLICE("some str1 foo foo foo foo foo"),
            .s2       = CHAR_SLICE("some str2 bar"),
            .dstCap   = 32,
            .wantDiff = "some str{1 f...|2 bar}",
        }
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(test); i++) {
        const test tt = tests[i];
        TEST_MESSAGE(tt.name);
        auto dst = CharSlice_New(Arena_Alloc(&mem, tt.dstCap), 0, tt.dstCap);
        CharSlice_Diff(&dst, tt.s1, tt.s2);

        TEST_ASSERT_EQUAL_STRING(tt.wantDiff, dst.arr);
    }
}

void Test_CharSlice_WriteF(void) {
    CharSlice dst1 = CharSlice_Make(0, 64);
    CharSlice dst2 = CharSlice_Make(0, 8);

    const auto printed1 = CharSlice_WriteF(&dst1, "test %d, %d, %d", 1, 2, 3);
    const auto printed2 = CharSlice_WriteF(&dst2, "test %d, %d, %d", 1, 2, 3);

    TEST_ASSERT_EQUAL_STRING("test 1, 2, 3", dst1.arr);
    TEST_ASSERT_EQUAL(12, printed1);
    TEST_ASSERT_EQUAL_STRING("test 1,", dst2.arr);
    TEST_ASSERT_EQUAL(7, printed2);
}

void Test_CharSlice_ReadWriteFile(void) {
    char       fileBuf[1024] = {};
    const auto in            = CHAR_SLICE("arbitrary text");
    CharSlice  out           = CharSlice_Make(0, 64);
    FILE*      f             = fmemopen(fileBuf, sizeof(fileBuf), "w+");

    File_WriteCharSlice(f, &in);
    fseek(f, 0, SEEK_SET);
    CharSlice_ReadFile(&out, f);

    TEST_ASSERT_EQUAL(0, CharSlice_Cmp(in, out));
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
    RUN_TEST(Test_CharSlice_Diff);
    RUN_TEST(Test_CharSlice_WriteF);
    RUN_TEST(Test_CharSlice_ReadWriteFile);

    printf("mem: %zd/%zd\n", mem.offset, mem.cap);

    return UNITY_END();
}
