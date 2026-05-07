#ifndef ARENA_H
#define ARENA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ARENA_GROW_FACTOR ((size_t)2)

typedef struct Arena {
    uint8_t*      buff;
    size_t        cap;
    size_t        offset;
    bool          onStack;
    struct Arena* next;
    bool          canGrow;
} Arena;

Arena* Arena_OnHeap(size_t cap, bool canGrow);
void*  Arena_Alloc(Arena* a, size_t size);
void   Arena_Reset(Arena* a);
void   Arena_Free(Arena* a);

#define Arena_OnStack(cap1) ((Arena){.buff = (uint8_t[(cap1)]){0}, .cap = (cap1), .onStack = true})

#endif  // ARENA_H
