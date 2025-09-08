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
    SIDE_PARSE_ERR_OK = 0,
    SIDE_PARSE_ERR_UNEXPECTED_VALUE,
} SideParseErr;

typedef struct {
    SideParseErr err;
    size_t       offset;
} SideParseResult;

typedef enum {
    PIECE_TYPE_PARSE_ERR_OK = 0,
    PIECE_TYPE_PARSE_ERR_INVALID_VALUE,
} PieceTypeParseErr;

typedef struct {
    PieceTypeParseErr err;
    size_t            offset;
} PieceTypeParseResult;

typedef enum {
    SQUARES_PARSE_ERR_OK = 0,
    SQUARES_PARSE_ERR_TOO_SHORT,
    SQUARES_PARSE_ERR_UNEXPECTED_CHAR,
} SquaresParseErr;

typedef struct {
    SquaresParseErr err;
    size_t          offset;
    char            unexpectedChar;
} SquaresParseResult;

char                 PieceType_ToUpperCaseChar(PieceType t);
char                 PieceType_ToLowerCaseChar(PieceType t);
bool                 PosParseResult_Equals(PosParseResult a, PosParseResult b);
bool                 MoveParseResult_Equals(MoveParseResult a, MoveParseResult b);
PieceTypeParseResult PieceType_Parse(PieceType* dst, Str src);
SideParseResult      Side_Parse(Side* dst, Str src);
bool                 Piece_FromChar(Piece* t, char ch);
const char*          Piece_ToUnicodeChar(Piece p);
PosParseResult       Pos_Parse(Pos* dst, Str src);
MoveParseResult      Move_Parse(Move* dst, Str src);
SquaresParseResult   Squares_Parse(Squares dst, Str src);
bool                 SquaresParseResult_Equals(SquaresParseResult a, SquaresParseResult b);
size_t               CharBuff_WritePieceType(CharBuff* dst, PieceType t);
size_t               CharBuff_WriteSide(CharBuff* dst, Side s);
size_t               CharBuff_WritePiece(CharBuff* dst, Piece p);
size_t               CharBuff_WritePosParseErr(CharBuff* dst, PosParseErr err);
size_t               CharBuff_WritePosParseResult(CharBuff* dst, PosParseResult a);
size_t               CharBuff_WritePos(CharBuff* dst, Pos a);
size_t               CharBuff_WriteMoveParseErr(CharBuff* dst, MoveParseErr err);
size_t               CharBuff_WriteMoveParseResult(CharBuff* dst, MoveParseResult a);
size_t               CharBuff_WriteMove(CharBuff* dst, Move a);
size_t               CharBuff_WriteBoardParseResult(CharBuff* dst, SquaresParseResult err);
size_t               CharBuff_WriteBoardParseErr(CharBuff* dst, SquaresParseErr err);
size_t               CharBuff_WriteMoveError(CharBuff* dst, MoveErr a);
size_t               CharBuff_WriteMoveResult(CharBuff* dst, const MoveResult* a);
size_t               CharBuff_WriteSquares(CharBuff* dst, const Squares ss);

#endif  // BOARD_REPR_H
