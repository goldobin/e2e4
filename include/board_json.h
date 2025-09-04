#ifndef BOARD_JSON_H
#define BOARD_JSON_H

#include "board.h"
#include "json.h"

// Interpret

bool Piece_InterpretJson(Piece* dst, JsonSource* src);
bool PieceTypes_InterpretJson(PieceTypes* dst, JsonSource* src);
bool SideState_InterpretJson(SideState* dst, JsonSource* src);
bool Squares_InterpretJson(Squares dst, JsonSource* src);
bool Board_InterpretJson(Board* dst, JsonSource* src);

// Write

size_t CharBuff_WritePieceAsJson(CharBuff* dst, JsonStack* js, Piece p);
size_t CharBuff_WriteSquaresAsJson(CharBuff* dst, JsonStack* js, const Squares src);
size_t CharBuff_WritePieceTypesAsJson(CharBuff* dst, JsonStack* js, PieceTypes src);
size_t CharBuff_WriteSideStateAsJson(CharBuff* dst, JsonStack* js, const SideState* src);
size_t CharBuff_WriteBoardAsJson(CharBuff* dst, JsonStack* js, const Board* src);

#endif  // BOARD_JSON_H
