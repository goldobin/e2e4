#include "arena.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "func.h"

Arena Arena_New(void* buffer, const size_t cap) {
    assert(buffer != nullptr);
    assert(cap > 0);

    return (Arena){.buff = (uint8_t*)buffer, .cap = cap, .offset = 0};
}

Arena Arena_NewAutoGrow(const size_t cap) {
    assert(cap > 0);
    return (Arena){
        .buff     = nullptr,
        .cap      = cap,
        .offset   = 0,
        .autoGrow = true,
    };
}

void* Arena_Alloc(Arena* a, const size_t size) {
    assert(a != nullptr);

    if (size < 1) {
        return nullptr;
    }

    if (!a->autoGrow) {
        if (a->offset + size > a->cap) {
            return nullptr;
        }

        if (a->buff == nullptr) {
            return nullptr;
        }

        const auto result = a->buff + a->offset;
        a->offset         = a->offset + size;
        return result;
    }

    Arena* cur      = a;
    size_t totalCap = 0;
    while (cur != nullptr) {
        if (cur->offset + size <= cur->cap) {
            break;
        }
        totalCap += cur->cap;
        if (cur->next != nullptr) {
            cur = cur->next;
            continue;
        }

        Arena* newBlock = malloc(sizeof(Arena));
        if (newBlock == nullptr) {
            return nullptr;
        }

        const auto newBlockCap = MaxSizeT(totalCap * ARENA_GROW_FACTOR, size);
        *newBlock              = Arena_NewAutoGrow(newBlockCap);
        cur->next              = newBlock;
        cur                    = newBlock;
    }

    if (cur->buff == nullptr) {
        cur->buff = malloc(cur->cap);
        if (cur->buff == nullptr) {
            return nullptr;
        }
    }

    const auto result = cur->buff + cur->offset;
    cur->offset       = cur->offset + size;
    return result;
}

void Arena_Reset(Arena* a) {
    assert(a != nullptr);
    a->offset = 0;

    Arena* cur = a;
    while (cur != nullptr) {
        memset(cur->buff, 0, cur->cap);
        cur->offset = 0;
        cur         = cur->next;
    }

    assert(cur != nullptr);
}

void Arena_Free(Arena* a) {
    assert(a != nullptr);

    if (!a->autoGrow) {
        // Noop
        return;
    }

    Arena* parent = a;
    Arena* child  = a->next;
    while (child != nullptr) {
        const auto block = child;
        parent->next     = nullptr;
        parent           = child;
        child            = child->next;

        free(block->buff);
        free(block);
    }

    free(a->buff);
    a->buff   = nullptr;
    a->cap    = 0;
    a->offset = 0;
}
