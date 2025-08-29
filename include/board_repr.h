#ifndef BOARD_REPR_H
#define BOARD_REPR_H

#include "board.h"

typedef enum {
    POS_PARSE_ERR_OK = 0,
    POS_PARSE_ERR_TOO_SHORT,
    POS_PARSE_ERR_INVALID_FORMAT,
} PosParseErr;

typedef struct {
    PosParseErr err;
    size_t      offset;
} PosParseResult;

typedef enum {
    MOVE_PARSE_ERR_OK = 0,
    MOVE_PARSE_ERR_TOO_SHORT,
    MOVE_PARSE_ERR_INVALID_FROM_FORMAT,
    MOVE_PARSE_ERR_INVALID_TO_FORMAT,
} MoveParseErr;

typedef struct {
    MoveParseErr err;
    size_t       offset;
} MoveParseResult;

typedef enum {
    BOARD_PARSE_ERR_OK = 0,
    BOARD_PARSE_ERR_TOO_SHORT,
    BOARD_PARSE_ERR_UNEXPECTED_CHAR,
} BoardParseErr;

typedef struct {
    BoardParseErr err;
    size_t        offset;
    char          unexpectedChar;
} BoardParseResult;

char             PieceType_ToUpperCaseChar(PieceType t);
char             PieceType_ToLowerCaseChar(PieceType t);
bool             PosParseResult_Equals(PosParseResult a, PosParseResult b);
bool             MoveParseResult_Equals(MoveParseResult a, MoveParseResult b);
PieceType        PieceType_Parse(CharSlice src);
Side             Side_Parse(CharSlice src);
bool             Piece_FromChar(Piece* t, char ch);
const char*      Piece_ToUnicodeChar(const Piece* p);
PosParseResult   Pos_Parse(Pos* dst, CharSlice src);
MoveParseResult  Move_Parse(Move* dst, CharSlice src);
bool             BoardParseResult_Equals(BoardParseResult a, BoardParseResult b);
BoardParseResult Board_Parse(Board* dst, CharSlice src);
size_t           CharSlice_WritePieceType(CharSlice* dst, PieceType t);
size_t           CharSlice_WriteSide(CharSlice* dst, Side s);
size_t           CharSlice_WritePiece(CharSlice* dst, Piece p);
size_t           CharSlice_WritePosParseErr(CharSlice* dst, PosParseErr err);
size_t           CharSlice_WritePosParseResult(CharSlice* dst, PosParseResult a);
size_t           CharSlice_WritePos(CharSlice* dst, Pos a);
size_t           CharSlice_WriteMoveParseErr(CharSlice* dst, MoveParseErr err);
size_t           CharSlice_WriteMoveParseResult(CharSlice* dst, MoveParseResult a);
size_t           CharSlice_WriteMove(CharSlice* dst, Move a);
size_t           CharSlice_WriteBoardParseResult(CharSlice* dst, BoardParseResult err);
size_t           CharSlice_WriteBoardParseErr(CharSlice* dst, BoardParseErr err);
size_t           CharSlice_WriteMoveError(CharSlice* dst, MoveErr a);
size_t           CharSlice_WriteMoveResult(CharSlice* dst, const MoveResult* a);
size_t           CharSlice_WriteBoard(CharSlice* dst, const Board* b);

#endif  // BOARD_REPR_H
