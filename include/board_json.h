#ifndef BOARD_JSON_H
#define BOARD_JSON_H

#include "board.h"
#include "json.h"

// Interpret

bool Piece_InterpretJson(Piece* dst, JsonSrc* src);
bool PieceTypes_InterpretJson(PieceTypes* dst, JsonSrc* src);
bool SideState_InterpretJson(SideState* dst, JsonSrc* src);
bool Squares_InterpretJson(Squares dst, JsonSrc* src);
bool Board_InterpretJson(Board* dst, JsonSrc* src);
bool Step_InterpretJson(Step* dst, JsonSrc* src);
bool Steps_InterpretJson(Steps* dst, JsonSrc* src);

// Write

size_t CharBuff_WriteMoveAsJson(CharBuff* dst, JsonStack* js, Move m);
size_t CharBuff_WriteSideAsJson(CharBuff* dst, JsonStack* js, Side s);
size_t CharBuff_WritePieceAsJson(CharBuff* dst, JsonStack* js, Piece p);
size_t CharBuff_WritePieceTypeAsJson(CharBuff* dst, JsonStack* js, PieceType t);
size_t CharBuff_WriteSquaresAsJson(CharBuff* dst, JsonStack* js, const Squares src);
size_t CharBuff_WritePieceTypesAsJson(CharBuff* dst, JsonStack* js, const PieceTypes* src);
size_t CharBuff_WriteSideStateAsJson(CharBuff* dst, JsonStack* js, const SideState* src);
size_t CharBuff_WriteStepAsJson(CharBuff* dst, JsonStack* js, const Step* src);
size_t CharBuff_WriteStepsAsJson(CharBuff* dst, JsonStack* js, const Steps* src);
size_t CharBuff_WriteBoardStateAsJson(CharBuff* dst, JsonStack* js, BoardState s);
size_t CharBuff_WriteBoardAsJson(CharBuff* dst, JsonStack* js, const Board* src);

#endif  // BOARD_JSON_H
