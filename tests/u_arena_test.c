#include <stdint.h>
#include <unity.h>

#include "u/arena.h"

void setUp(void) {}

void tearDown(void) {}

void test_Arena_Init_NonZeroEmptyBuffer(void) {
    uint8_t     buffer[512] = {};
    const Arena arena       = Arena_Wrap(buffer);

    TEST_ASSERT_EQUAL_PTR(buffer, arena.m);
    TEST_ASSERT_EQUAL_size_t(sizeof(buffer), arena.cap);
    TEST_ASSERT_EQUAL_size_t(0, arena.offset);
}

void test_Arena_Allocate_Consecutively(void) {
    uint8_t buffer[32] = {};
    Arena   arena      = Arena_Wrap(buffer);

    uint8_t* a1 = Arena_Alloc(&arena, sizeof(uint8_t) * 4);

    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[4]){}, a1, 4);
    TEST_ASSERT_EQUAL_PTR(buffer, a1);

    a1[0] = 1;
    a1[1] = 2;
    a1[2] = 3;
    a1[3] = 4;

    uint8_t* a2 = Arena_Alloc(&arena, sizeof(uint8_t) * 4);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[4]){}, a2, 4);
    TEST_ASSERT_EQUAL_PTR(buffer + 4, a2);

    a2[0] = 5;
    a2[1] = 6;
    a2[2] = 7;
    a2[3] = 8;

    const uint8_t expectedBuffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 0, 0};
    TEST_ASSERT_EQUAL_INT8_ARRAY(expectedBuffer, buffer, 10);

    for (size_t i = 10; i < 32; i++) {
        TEST_ASSERT_EQUAL_INT8(0, buffer[i]);
    }
    TEST_ASSERT_EQUAL(8, arena.offset);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_Arena_Init_NonZeroEmptyBuffer);
    RUN_TEST(test_Arena_Allocate_Consecutively);

    return UNITY_END();
}
