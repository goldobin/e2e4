#ifndef ARENA_H
#define ARENA_H
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

Arena Arena_New(void* buffer, size_t cap);
Arena Arena_NewAutoGrow(size_t cap);
void* Arena_Alloc(Arena* a, size_t size);
void  Arena_Reset(Arena* a);
void  Arena_Free(Arena* a);

#define Arena_Wrap(buffer) Arena_New(buffer, sizeof(buffer))

#endif  // ARENA_H
