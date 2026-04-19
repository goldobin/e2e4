#ifndef ARENA_H
#define ARENA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

constexpr size_t ARENA_GROW_FACTOR = 2;

typedef struct Arena {
    uint8_t*      buff;
    size_t        cap;
    size_t        offset;
    struct Arena* next;
    bool          autoGrow;
} Arena;

Arena Arena_OnHeap(size_t cap, bool autoGrow);
void* Arena_Alloc(Arena* a, size_t size);
void  Arena_Reset(Arena* a);
void  Arena_Free(Arena* a);

#define Arena_OnStack(cap1) ((Arena){.buff = (uint8_t[(cap1)]){}, .cap = (cap1)})

#endif  // ARENA_H
