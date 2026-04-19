#include "../include/chars.h"

#include <assert.h>
#include <stdlib.h>
#include <unity.h>

#include "../include/arena.h"

auto mem = Arena_OnStack(512);

void setUp(void) {}
void tearDown(void) {}

void Test_Str_At(void) {
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

void Test_CharBuff_WriteOne(void) {
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

void Test_CharBuff_WriteChar(void) {
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

void Test_Str_View(void) {
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

void Test_CharBuff_WriteF(void) {
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

    TEST_ASSERT_TRUE(CharBuff_Eq(in, out));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(Test_Str_At);
    RUN_TEST(Test_CharBuff_WriteOne);
    RUN_TEST(Test_CharBuff_WriteChar);
    RUN_TEST(Test_CharBuff_Write);
    RUN_TEST(Test_Str_View);
    RUN_TEST(Test_CharBuff_EdgeCases);
    RUN_TEST(Test_CharBuff_WriteF);
    RUN_TEST(Test_CharBuff_ReadWriteFile);

    printf("mem: %zd/%zd\n", mem.offset, mem.cap);

    return UNITY_END();
}
