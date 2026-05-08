#include "../include/arena.h"

#include <stdint.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_Arena_Init_NonZeroEmptyBuffer(void) {
    const Arena arena = Arena_OnStack(512);

    TEST_ASSERT_EQUAL(512, arena.cap);
    TEST_ASSERT_EQUAL(0, arena.offset);
}

void test_Arena_Allocate_Consecutively(void) {
    Arena    arena = Arena_OnStack(32);
    uint8_t* a1    = Arena_Alloc(&arena, sizeof(char) * 4);

    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[4]){0}, a1, 4);
    TEST_ASSERT_EQUAL_PTR(arena.buff, a1);

    a1[0] = '1';
    a1[1] = '2';
    a1[2] = '3';
    a1[3] = '4';

    uint8_t* a2 = Arena_Alloc(&arena, sizeof(char) * 4);
    TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t[4]){0}, a2, 4);
    TEST_ASSERT_EQUAL_PTR(arena.buff + 4, a2);

    a2[0] = '5';
    a2[1] = '6';
    a2[2] = '7';
    a2[3] = '8';

    const uint8_t expectedBuffer[10] = {'1', '2', '3', '4', '5', '6', '7', '8', 0, 0};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedBuffer, arena.buff, 10);

    for (size_t i = 10; i < 32; i++) {
        TEST_ASSERT_EQUAL_INT8(0, arena.buff[i]);
    }
    TEST_ASSERT_EQUAL(8, arena.offset);
}

void test_Arena_Grow1(void) {
    Arena* arena = Arena_OnHeap(32, true);
    TEST_ASSERT_NOT_NULL(arena);
    char* a1 = Arena_Alloc(arena, sizeof(char) * 30);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[30]){0}, a1, 30);

    a1[0] = '1';
    a1[1] = '2';
    a1[2] = '3';
    a1[3] = '4';

    char* a2 = Arena_Alloc(arena, sizeof(char) * 4);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[4]){0}, a2, 4);
    TEST_ASSERT_NOT_NULL(arena->next);
    TEST_ASSERT_NULL(arena->next->next);

    a2[0] = '5';
    a2[1] = '6';
    a2[2] = '7';
    a2[3] = '8';

    Arena_Free(arena);
}

void test_Arena_Grow2(void) {
    Arena* arena = Arena_OnHeap(32, true);
    TEST_ASSERT_NOT_NULL(arena);
    const char* a1 = Arena_Alloc(arena, sizeof(char) * 128);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[128]){0}, a1, 128);

    const char* a2 = Arena_Alloc(arena, sizeof(char) * 16);
    TEST_ASSERT_EQUAL_INT8_ARRAY((uint8_t[16]){0}, a2, 16);

    Arena_Free(arena);
}

void test_Arena_OnStack_OutOfSpace(void) {
    Arena          arena = Arena_OnStack(8);
    const uint8_t* a1    = Arena_Alloc(&arena, 4);
    TEST_ASSERT_NOT_NULL(a1);
    TEST_ASSERT_EQUAL(4, arena.offset);

    const uint8_t* a2 = Arena_Alloc(&arena, 8);
    TEST_ASSERT_NULL(a2);
    TEST_ASSERT_EQUAL(4, arena.offset);
}

void test_Arena_OnHeap_NoGrow_CapsOut(void) {
    Arena* arena = Arena_OnHeap(32, false);
    TEST_ASSERT_NOT_NULL(arena);

    const void* a1 = Arena_Alloc(arena, 24);
    TEST_ASSERT_NOT_NULL(a1);

    const void* a2 = Arena_Alloc(arena, 16);
    TEST_ASSERT_NULL(a2);
    TEST_ASSERT_NULL(arena->next);

    const void* a3 = Arena_Alloc(arena, 8);
    TEST_ASSERT_NOT_NULL(a3);

    Arena_Free(arena);
}

void test_Arena_Alloc_OverflowSafe(void) {
    Arena       stackArena = Arena_OnStack(32);
    const void* a1         = Arena_Alloc(&stackArena, SIZE_MAX);
    TEST_ASSERT_NULL(a1);
    TEST_ASSERT_EQUAL(0, stackArena.offset);

    Arena* heapArena = Arena_OnHeap(32, false);
    TEST_ASSERT_NOT_NULL(heapArena);

    const void* a2 = Arena_Alloc(heapArena, SIZE_MAX);
    TEST_ASSERT_NULL(a2);
    Arena_Free(heapArena);
}

void test_Arena_Grow_NewBlockZeroed(void) {
    Arena* arena = Arena_OnHeap(8, true);
    TEST_ASSERT_NOT_NULL(arena);

    uint8_t* a1 = Arena_Alloc(arena, 8);
    TEST_ASSERT_NOT_NULL(a1);
    for (size_t i = 0; i < 8; i++) {
        a1[i] = 0xFF;
    }

    uint8_t* a2 = Arena_Alloc(arena, 8);
    TEST_ASSERT_NOT_NULL(a2);
    TEST_ASSERT_NOT_NULL(arena->next);
    TEST_ASSERT_EQUAL_PTR(arena->next->buff, a2);
    TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t[8]){0}, a2, 8);

    Arena_Free(arena);
}

void test_Arena_Reset_MultiBlock(void) {
    Arena* arena = Arena_OnHeap(8, true);
    TEST_ASSERT_NOT_NULL(arena);

    uint8_t* a1 = Arena_Alloc(arena, 8);
    for (size_t i = 0; i < 8; i++) {
        a1[i] = 0xAA;
    }

    uint8_t* a2 = Arena_Alloc(arena, 8);
    for (size_t i = 0; i < 8; i++) {
        a2[i] = 0xBB;
    }
    TEST_ASSERT_NOT_NULL(arena->next);

    Arena_Reset(arena);

    TEST_ASSERT_EQUAL(0, arena->offset);
    TEST_ASSERT_EQUAL(0, arena->next->offset);
    TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t[8]){0}, arena->buff, 8);
    TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t[8]){0}, arena->next->buff, 8);

    uint8_t* a3 = Arena_Alloc(arena, 4);
    TEST_ASSERT_EQUAL_PTR(arena->buff, a3);

    Arena_Free(arena);
}

void test_Arena_Free_OnStackIsNoop(void) {
    Arena       arena = Arena_OnStack(32);
    const void* a1    = Arena_Alloc(&arena, 8);
    TEST_ASSERT_NOT_NULL(a1);

    Arena_Free(&arena);
}

void test_Arena_Grow_ChainMultiSteps(void) {
    Arena* arena = Arena_OnHeap(8, true);
    TEST_ASSERT_NOT_NULL(arena);

    const void* a1 = Arena_Alloc(arena, 4);
    TEST_ASSERT_NOT_NULL(a1);
    TEST_ASSERT_NULL(arena->next);

    const void* a2 = Arena_Alloc(arena, 8);
    TEST_ASSERT_NOT_NULL(a2);
    TEST_ASSERT_NOT_NULL(arena->next);
    TEST_ASSERT_NULL(arena->next->next);

    const void* a3 = Arena_Alloc(arena, 32);
    TEST_ASSERT_NOT_NULL(a3);
    TEST_ASSERT_NOT_NULL(arena->next->next);
    TEST_ASSERT_NULL(arena->next->next->next);

    Arena_Free(arena);
}

void test_Arena_Reset_RefillHeadExactly(void) {
    Arena* arena = Arena_OnHeap(16, true);
    TEST_ASSERT_NOT_NULL(arena);

    const void* a1 = Arena_Alloc(arena, 10);
    TEST_ASSERT_NOT_NULL(a1);
    const void* a2 = Arena_Alloc(arena, 10);
    TEST_ASSERT_NOT_NULL(a2);
    TEST_ASSERT_NOT_NULL(arena->next);

    Arena_Reset(arena);
    TEST_ASSERT_EQUAL(0, arena->offset);

    const void* a3 = Arena_Alloc(arena, 16);
    TEST_ASSERT_NOT_NULL(a3);
    TEST_ASSERT_EQUAL_PTR(arena->buff, a3);
    TEST_ASSERT_EQUAL(16, arena->offset);
    TEST_ASSERT_EQUAL(0, arena->next->offset);

    Arena_Free(arena);
}

void test_Arena_Grow_CanGrowPropagates(void) {
    Arena* growing = Arena_OnHeap(8, true);
    TEST_ASSERT_NOT_NULL(growing);
    TEST_ASSERT_TRUE(growing->canGrow);

    TEST_ASSERT_NOT_NULL(Arena_Alloc(growing, 8));
    TEST_ASSERT_NOT_NULL(Arena_Alloc(growing, 8));
    TEST_ASSERT_NOT_NULL(growing->next);
    TEST_ASSERT_TRUE(growing->next->canGrow);

    TEST_ASSERT_NOT_NULL(Arena_Alloc(growing, 64));
    TEST_ASSERT_NOT_NULL(growing->next->next);
    TEST_ASSERT_TRUE(growing->next->next->canGrow);

    Arena_Free(growing);

    Arena* fixed = Arena_OnHeap(8, false);
    TEST_ASSERT_NOT_NULL(fixed);
    TEST_ASSERT_FALSE(fixed->canGrow);
    TEST_ASSERT_NOT_NULL(Arena_Alloc(fixed, 8));
    TEST_ASSERT_NULL(Arena_Alloc(fixed, 1));
    TEST_ASSERT_NULL(fixed->next);
    Arena_Free(fixed);
}

void test_Arena_Boundary_ExactCap(void) {
    Arena       stackArena = Arena_OnStack(16);
    const void* s1         = Arena_Alloc(&stackArena, 16);
    TEST_ASSERT_NOT_NULL(s1);
    TEST_ASSERT_EQUAL(16, stackArena.offset);

    const void* s2 = Arena_Alloc(&stackArena, 1);
    TEST_ASSERT_NULL(s2);

    Arena* heapArena = Arena_OnHeap(16, false);
    TEST_ASSERT_NOT_NULL(heapArena);
    const void* h1 = Arena_Alloc(heapArena, 16);
    TEST_ASSERT_NOT_NULL(h1);

    const void* h2 = Arena_Alloc(heapArena, 1);
    TEST_ASSERT_NULL(h2);

    Arena_Free(heapArena);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_Arena_Init_NonZeroEmptyBuffer);
    RUN_TEST(test_Arena_Allocate_Consecutively);
    RUN_TEST(test_Arena_Grow1);
    RUN_TEST(test_Arena_Grow2);
    RUN_TEST(test_Arena_OnStack_OutOfSpace);
    RUN_TEST(test_Arena_OnHeap_NoGrow_CapsOut);
    RUN_TEST(test_Arena_Alloc_OverflowSafe);
    RUN_TEST(test_Arena_Grow_NewBlockZeroed);
    RUN_TEST(test_Arena_Reset_MultiBlock);
    RUN_TEST(test_Arena_Free_OnStackIsNoop);
    RUN_TEST(test_Arena_Grow_ChainMultiSteps);
    RUN_TEST(test_Arena_Reset_RefillHeadExactly);
    RUN_TEST(test_Arena_Grow_CanGrowPropagates);
    RUN_TEST(test_Arena_Boundary_ExactCap);

    return UNITY_END();
}
