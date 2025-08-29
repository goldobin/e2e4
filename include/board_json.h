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

size_t CharSlice_WritePieceAsJson(CharSlice* dst, JsonStack* js, const Piece* p);
size_t CharSlice_WriteSquaresAsJson(CharSlice* dst, JsonStack* js, const Squares src);
size_t CharSlice_WritePieceTypeArrAsJson(CharSlice* dst, JsonStack* js, const PieceTypes* src);
size_t CharSlice_WriteSideStateAsJson(CharSlice* dst, JsonStack* js, const SideState* src);
size_t CharSlice_WriteBoardAsJson(CharSlice* dst, JsonStack* js, const Board* src);

#endif  // BOARD_JSON_H
