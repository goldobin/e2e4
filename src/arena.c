#include "arena.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "func.h"

Arena Arena_OnHeap(const size_t cap, const bool autoGrow) {
    assert(cap > 0);
    return (Arena){
        .buff     = NULL,
        .cap      = cap,
        .offset   = 0,
        .autoGrow = autoGrow,
    };
}

void* Arena_Alloc(Arena* a, const size_t size) {
    assert(a != NULL);
    assert(size > 0);

    if (!a->autoGrow) {
        if (a->offset + size > a->cap) {
            return NULL;
        }

        if (a->buff == NULL) {
            return NULL;
        }

        uint8_t* const result = a->buff + a->offset;
        a->offset             = a->offset + size;
        return result;
    }

    Arena* cur      = a;
    size_t totalCap = 0;
    while (cur != NULL) {
        if (cur->offset + size <= cur->cap) {
            break;
        }
        totalCap += cur->cap;
        if (cur->next != NULL) {
            cur = cur->next;
            continue;
        }

        Arena* newBlock = malloc(sizeof(Arena));
        if (newBlock == NULL) {
            return NULL;
        }

        const size_t newBlockCap = MaxSizeT(totalCap * ARENA_GROW_FACTOR, size);
        *newBlock                = Arena_OnHeap(newBlockCap, cur->autoGrow);
        cur->next                = newBlock;
        cur                      = newBlock;
    }

    if (cur->buff == NULL) {
        cur->buff = malloc(cur->cap);
        if (cur->buff == NULL) {
            return NULL;
        }
    }

    uint8_t* const result = cur->buff + cur->offset;
    cur->offset           = cur->offset + size;
    return result;
}

void Arena_Reset(Arena* a) {
    assert(a != NULL);
    a->offset = 0;

    Arena* cur = a;
    while (cur != NULL) {
        memset(cur->buff, 0, cur->cap);
        cur->offset = 0;
        cur         = cur->next;
    }
}

void Arena_Free(Arena* a) {
    assert(a != NULL);

    if (!a->autoGrow) {
        // Noop
        return;
    }

    Arena* parent = a;
    Arena* child  = a->next;
    while (child != NULL) {
        Arena* const block = child;
        parent->next       = NULL;
        parent             = child;
        child              = child->next;

        free(block->buff);
        free(block);
    }

    free(a->buff);
    a->buff   = NULL;
    a->cap    = 0;
    a->offset = 0;
}
