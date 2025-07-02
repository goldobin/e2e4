#include "../include/arena.h"

#include <stdint.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_Arena_Init_NonZeroEmptyBuffer(void) {
    uint8_t     buffer[512] = {};
    const Arena arena       = Arena_Wrap(buffer);

    TEST_ASSERT_EQUAL_PTR(buffer, arena.buff);
    TEST_ASSERT_EQUAL_size_t(sizeof(buffer), arena.cap);
    TEST_ASSERT_EQUAL_size_t(0, arena.offset);
}

void test_Arena_Allocate_Consecutively(void) {
    uint8_t buffer[32] = {};
    Arena   arena      = Arena_Wrap(buffer);

    char* a1 = Arena_Alloc(&arena, sizeof(char) * 4);

    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[4]){}, a1, 4);
    TEST_ASSERT_EQUAL_PTR(buffer, a1);

    a1[0] = '1';
    a1[1] = '2';
    a1[2] = '3';
    a1[3] = '4';

    char* a2 = Arena_Alloc(&arena, sizeof(char) * 4);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[4]){}, a2, 4);
    TEST_ASSERT_EQUAL_PTR(buffer + 4, a2);

    a2[0] = '5';
    a2[1] = '6';
    a2[2] = '7';
    a2[3] = '8';

    constexpr uint8_t expectedBuffer[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '0', '0'};
    TEST_ASSERT_EQUAL_INT8_ARRAY(expectedBuffer, buffer, 10);

    for (size_t i = 10; i < 32; i++) {
        TEST_ASSERT_EQUAL_INT8(0, buffer[i]);
    }
    TEST_ASSERT_EQUAL(8, arena.offset);
}

void test_Arena_Grow1(void) {
    Arena arena = Arena_NewAutoGrow(32);
    char* a1    = Arena_Alloc(&arena, sizeof(char) * 30);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[30]){}, a1, 30);

    a1[0] = '1';
    a1[1] = '2';
    a1[2] = '3';
    a1[3] = '4';

    char* a2 = Arena_Alloc(&arena, sizeof(char) * 4);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[4]){}, a2, 4);
    TEST_ASSERT_NOT_NULL(arena.next);
    TEST_ASSERT_NULL(arena.next->next);

    a2[0] = '5';
    a2[1] = '6';
    a2[2] = '7';
    a2[3] = '8';

    Arena_Free(&arena);
    TEST_ASSERT_NULL(arena.buff);
    TEST_ASSERT_EQUAL(0, arena.cap);
    TEST_ASSERT_EQUAL(0, arena.offset);
}

void test_Arena_Grow2(void) {
    Arena       arena = Arena_NewAutoGrow(32);
    const char* a1    = Arena_Alloc(&arena, sizeof(char) * 128);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[128]){}, a1, 128);

    const char* a2 = Arena_Alloc(&arena, sizeof(char) * 16);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[16]){}, a2, 16);

    Arena_Free(&arena);
    TEST_ASSERT_NULL(arena.buff);
    TEST_ASSERT_EQUAL(0, arena.cap);
    TEST_ASSERT_EQUAL(0, arena.offset);
}

int main(void) {
    UNITY_BEGIN();

    // RUN_TEST(test_Arena_Init_NonZeroEmptyBuffer);
    // RUN_TEST(test_Arena_Allocate_Consecutively);
    RUN_TEST(test_Arena_Grow2);

    return UNITY_END();
}
