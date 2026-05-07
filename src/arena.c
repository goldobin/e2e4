#include "arena.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "func.h"

Arena* Arena_OnHeap(const size_t cap, const bool canGrow) {
    assert(cap > 0);
    Arena* result = malloc(sizeof(Arena));
    if (result == NULL) {
        return NULL;
    }
    *result = (Arena){
        .buff    = NULL,
        .cap     = cap,
        .offset  = 0,
        .canGrow = canGrow,
    };
    return result;
}

void* Arena_Alloc(Arena* a, const size_t size) {
    assert(a != NULL);
    assert(size > 0);

    if (a->onStack) {
        if (size > a->cap - a->offset) {
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
        if (size <= cur->cap - cur->offset) {
            break;
        }
        if (!cur->canGrow) {
            return NULL;
        }
        totalCap += cur->cap;
        if (cur->next != NULL) {
            cur = cur->next;
            continue;
        }

        const size_t nextBlockCap = MaxSizeT(totalCap * ARENA_GROW_FACTOR, size);
        Arena*       nextBlock    = Arena_OnHeap(nextBlockCap, cur->canGrow);
        if (nextBlock == NULL) {
            return NULL;
        }
        cur->next = nextBlock;
        cur       = nextBlock;
    }

    assert(cur != NULL);
    if (cur->buff == NULL) {
        cur->buff = calloc(1, cur->cap);
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

    Arena* cur = a;
    while (cur != NULL) {
        if (cur->buff != NULL) {
            memset(cur->buff, 0, cur->cap);
        }
        cur->offset = 0;
        cur         = cur->next;
    }
}

void Arena_Free(Arena* a) {
    assert(a != NULL);
    if (a->onStack) {
        return;
    }
    Arena* cur = a;
    while (cur != NULL) {
        Arena* prev = cur;
        cur         = cur->next;
        free(prev->buff);
        free(prev);
    }
}
