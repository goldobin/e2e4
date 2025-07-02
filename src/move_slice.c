#include "move_slice.h"

#include <assert.h>
#include <string.h>

#include "board.h"
#include "func.h"

inline MoveSlice MoveSlice_New(Move* buffer, const size_t len, const size_t cap) {
    assert(buffer != nullptr);
    assert(len <= cap);

    return (MoveSlice){
        .items = buffer,
        .len   = len,
        .cap   = cap,
    };
}

MoveSlice MoveSlice_NewAutoGrow(Arena* a, const size_t len, size_t cap) {
    assert(a != nullptr);
    assert(len <= cap);
    Move* items = Arena_Alloc(a, cap * sizeof(Move));
    return (MoveSlice){.items = items, .len = len, .cap = cap, .a = a};
}

const Move* MoveSlice_At(const MoveSlice* s, const size_t i) {
    assert(s != nullptr);
    assert(i < s->len);
    return &s->items[i];
}

MoveSlice MoveSlice_View(const MoveSlice* s, const size_t start, const size_t end) {
    assert(s != nullptr);
    assert(start <= end);
    assert(end <= s->len);
    const auto len = end - start;
    return (MoveSlice){.items = &s->items[start], .cap = len, .len = len, .a = s->a};
}

size_t MoveSlice_WriteOneAt(MoveSlice* dst, const size_t offset, const Move ch) {
    assert(dst != nullptr);
    if (offset > dst->len) {
        return 0;
    }

    if (offset == dst->cap) {
        if (dst->a == nullptr) {
            return 0;
        }

        if (!MoveSlice_Reserve(dst, dst->cap * MOVE_SLICE_GROW_FACTOR)) {
            return 0;
        }
    }

    dst->items[offset] = ch;
    if (offset == dst->len) {
        dst->len++;
    }
    return 1;
}

size_t MoveSlice_WriteAt(MoveSlice* dst, size_t offset, MoveSlice other) {
    assert(dst != nullptr);
    if (offset > dst->len) {
        return 0;
    }

    if (offset >= dst->cap) {
        if (dst->a == nullptr) {
            return 0;
        }

        if (!MoveSlice_Reserve(dst, dst->cap * MOVE_SLICE_GROW_FACTOR)) {
            return 0;
        }
    }

    const auto itemsToCopy = MinSizeT(dst->cap - offset, other.len);
    if (itemsToCopy == 0) {
        return 0;
    }

    memcpy(&dst->items[offset], &other.items[0], itemsToCopy);
    dst->len = offset + itemsToCopy;
    return itemsToCopy;
}

size_t MoveSlice_WriteOne(MoveSlice* dst, const Move v) {
    assert(dst != nullptr);
    return MoveSlice_WriteOneAt(dst, dst->len, v);
}

size_t MoveSlice_Write(MoveSlice* dst, const MoveSlice other) {
    assert(dst != nullptr);
    return MoveSlice_WriteAt(dst, dst->len, other);
}

bool MoveSlice_Reserve(MoveSlice* dst, const size_t cap) {
    assert(dst != nullptr);
    if (cap <= dst->cap) {
        return true;
    }

    Move* items = Arena_Alloc(dst->a, cap * sizeof(Move));
    if (items == nullptr) {
        return false;
    }

    memcpy(items, dst->items, dst->len * sizeof(Move));
    dst->items = items;
    dst->cap   = cap;
    return true;
}
