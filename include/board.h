#ifndef BOARD_H
#define BOARD_H

#include "char_slice.h"

constexpr size_t BOARD_SIDE_LEN = 8;
constexpr size_t BOARD_SIZE     = BOARD_SIDE_LEN * BOARD_SIDE_LEN;
constexpr char   COL_CHAR_MIN   = 'a';
constexpr char   COL_CHAR_MAX   = COL_CHAR_MIN + BOARD_SIDE_LEN - 1;
constexpr char   ROW_CHAR_MIN   = '1';
constexpr char   ROW_CHAR_MAX   = ROW_CHAR_MIN + BOARD_SIDE_LEN - 1;
constexpr size_t POS_STR_LEN    = 2;
constexpr size_t MOVE_STR_LEN   = 2 * POS_STR_LEN;
constexpr size_t THREATS_CAP    = 16;

// White pieces
constexpr char UNICODE_WHITE_KING[4]   = "\u2654\0";
constexpr char UNICODE_WHITE_QUEEN[4]  = "\u2655\0";
constexpr char UNICODE_WHITE_ROOK[4]   = "\u2656\0";
constexpr char UNICODE_WHITE_BISHOP[4] = "\u2657\0";
constexpr char UNICODE_WHITE_KNIGHT[4] = "\u2658\0";
constexpr char UNICODE_WHITE_PAWN[4]   = "\u2659\0";

// Black pieces
constexpr char UNICODE_BLACK_KING[4]   = "\u265A\0";
constexpr char UNICODE_BLACK_QUEEN[4]  = "\u265B\0";
constexpr char UNICODE_BLACK_ROOK[4]   = "\u265C\0";
constexpr char UNICODE_BLACK_BISHOP[4] = "\u265D\0";
constexpr char UNICODE_BLACK_KNIGHT[4] = "\u265E\0";
constexpr char UNICODE_BLACK_PAWN[4]   = "\u265F\0";

typedef enum {
    PIECE_TYPE_NONE = 0,
    PIECE_TYPE_PAWN,
    PIECE_TYPE_ROOK,
    PIECE_TYPE_KNIGHT,
    PIECE_TYPE_BISHOP,
    PIECE_TYPE_QUEEN,
    PIECE_TYPE_KING
} PieceType;

char PieceType_ToUpperCaseChar(PieceType t);
char PieceType_ToLowerCaseChar(PieceType t);
bool PieceType_IsValid(PieceType t);

typedef enum {
    SIDE_NONE = 0,
    SIDE_BLACK,
    SIDE_WHITE
} Side;

typedef struct {
    Side      side;
    PieceType type;
} Piece;

bool        Piece_IsEmpty(Piece p);
bool        Piece_Equals(Piece a, Piece b);
bool        Piece_FromChar(Piece* t, char ch);
const char* Piece_ToUnicodeChar(Piece p);

typedef struct {
    size_t col;
    size_t row;
} Pos;

typedef struct {
    int x;
    int y;
} Vec2I;

Vec2I Vec2I_FromPos(Pos l, Pos r);

typedef enum {
    POS_PARSE_ERR_OK = 0,
    POS_PARSE_ERR_TOO_SHORT,
    POS_PARSE_ERR_INVALID_FORMAT,
} PosParseErr;

typedef struct {
    PosParseErr err;
    size_t      offset;
} PosParseResult;

bool           PosParseResult_Equals(PosParseResult a, PosParseResult b);
PosParseResult Pos_Parse(Pos* dst, CharSlice src);
bool           Pos_Equals(Pos a, Pos b);
bool           Pos_IsValid(Pos a);
Side           Pos_BoardSide(Pos a);

typedef struct {
    Pos from;
    Pos to;
} Move;

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

bool            MoveParseResult_Equals(MoveParseResult a, MoveParseResult b);
MoveParseResult Move_Parse(Move* dst, CharSlice src);
bool            Move_Equals(Move a, Move b);
bool            Move_IsValid(Move a);

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

bool BoardParseResult_Equals(BoardParseResult a, BoardParseResult b);

typedef enum {
    MOVE_ERR_OK = 0,
    MOVE_ERR_NO_PIECE,
    MOVE_ERR_NO_MOVEMENT,
    MOVE_ERR_ILLEGAL,
    MOVE_ERR_UNDER_THREAT,
    MOVE_ERR_OBSTACLE
} MoveErr;

typedef struct {
    MoveErr err;
    Pos     obstacleAt;
    Piece   pieceTaken;
} MoveResult;

bool MoveResult_Equals(const MoveResult* a, const MoveResult* b);

typedef struct {
    Piece m[BOARD_SIDE_LEN][BOARD_SIDE_LEN];
    bool  whiteKingCastled;
    bool  blackKingCastled;
} Board;

void             Board_PlacePieces(Board* dst);
bool             Board_Equals(const Board* a, const Board* b);
BoardParseResult Board_Parse(Board* dst, CharSlice src);
Piece            Board_At(const Board* b, Pos pos);
void             Board_UpdateAt(Board* b, Pos pos, Piece p);
MoveResult       Board_MakeMove(Board* b, Move m);
MoveResult       Board_CheckMove(const Board* b, Move m);
MoveResult       Board_CheckPawnMove(const Board* b, Move m);
MoveResult       Board_CheckRookMove(const Board* b, Move m);
MoveResult       Board_CheckKnightMove(const Board* b, Move m);
MoveResult       Board_CheckBishopMove(const Board* b, Move m);
MoveResult       Board_CheckQueenMove(const Board* b, Move m);
MoveResult       Board_CheckKingMove(const Board* b, Move m);
void             Board_Copy(Board* dst, const Board* src);

typedef struct {
    Pos       pos;
    PieceType pieceType;
} Threat;

bool Threat_IsValid(Threat t);

typedef struct {
    size_t len;
    Threat items[THREATS_CAP];
} Threats;

size_t Threats_Append(Threats* ts, Threat t);
size_t Threats_Collect(Threats* dst, const Board* b, Pos p);

size_t CharSlice_WritePieceType(CharSlice* dst, PieceType t);
size_t CharSlice_WriteSide(CharSlice* dst, Side s);
size_t CharSlice_WritePiece(CharSlice* dst, Piece p);
size_t CharSlice_WritePosParseErr(CharSlice* dst, PosParseErr err);
size_t CharSlice_WritePosParseResult(CharSlice* dst, PosParseResult a);
size_t CharSlice_WritePos(CharSlice* dst, Pos a);
size_t CharSlice_WriteMoveParseErr(CharSlice* dst, MoveParseErr err);
size_t CharSlice_WriteMoveParseResult(CharSlice* dst, MoveParseResult a);
size_t CharSlice_WriteMove(CharSlice* dst, Move a);
size_t CharSlice_WriteBoardParseResult(CharSlice* dst, BoardParseResult err);
size_t CharSlice_WriteBoardParseErr(CharSlice* dst, BoardParseErr err);
size_t CharSlice_WriteMoveError(CharSlice* dst, MoveErr a);
size_t CharSlice_WriteMoveResult(CharSlice* dst, const MoveResult* a);
size_t CharSlice_WriteBoard(CharSlice* dst, const Board* b);

#endif  // BOARD_H
