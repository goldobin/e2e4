#ifndef MOVE_SLICE_H
#define MOVE_SLICE_H

#include "arena.h"
#include "board.h"

constexpr size_t MOVE_SLICE_GROW_FACTOR = 2;

typedef struct {
    Move*  items;
    size_t len;
    size_t cap;
    Arena* a;
} MoveSlice;

MoveSlice   MoveSlice_New(Move* buffer, size_t len, size_t cap);
MoveSlice   MoveSlice_NewAutoGrow(Arena* a, size_t len, size_t cap);
const Move* MoveSlice_At(const MoveSlice* s, size_t i);
MoveSlice   MoveSlice_View(const MoveSlice* s, size_t start, size_t end);
bool        MoveSlice_Reserve(MoveSlice* dst, size_t cap);
size_t      MoveSlice_WriteOneAt(MoveSlice* dst, size_t offset, Move ch);
size_t      MoveSlice_WriteAt(MoveSlice* dst, size_t offset, MoveSlice other);
size_t      MoveSlice_WriteOne(MoveSlice* dst, Move v);
size_t      MoveSlice_Write(MoveSlice* dst, MoveSlice other);

#define MoveSlice_Wrap(buffer) MoveSlice_New(buffer, 0, sizeof(buffer))

#endif  // MOVE_SLICE_H
