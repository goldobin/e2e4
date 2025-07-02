#ifndef U_ARENA_H
#define U_ARENA_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* m;
    size_t   cap;
    size_t   offset;
} Arena;

typedef struct {
    size_t used;
    size_t remaining;
} ArenaStats;

Arena Arena_New(void* buffer, size_t cap);
void* Arena_Alloc(Arena* a, size_t size);
void  Arena_Reset(Arena* a);

#define Arena_Wrap(buffer) Arena_New(buffer, sizeof(buffer))

#endif  // U_ARENA_H
