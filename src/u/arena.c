#include "u/arena.h"

#include <assert.h>

Arena Arena_New(void* buffer, const size_t cap) {
    assert(buffer != nullptr);
    assert(cap > 0);

    return (Arena){.m = (uint8_t*)buffer, .cap = cap, .offset = 0};
}

void* Arena_Alloc(Arena* a, const size_t size) {
    assert(a != nullptr);
    assert(size > 0);

    if (a->offset + size > a->cap) {
        return nullptr;  // Out of memory
    }

    const auto result = &a->m[a->offset];
    // Update offset and return pointer
    a->offset = a->offset + size;
    return result;
}

void Arena_Reset(Arena* a) {
    assert(a != nullptr);
    a->offset = 0;
}

ArenaStats Arena_Stats(const Arena* a) {
    assert(a != nullptr);
    return (ArenaStats){
        .remaining = a->cap - a->offset,
        .used      = a->offset,
    };
}
