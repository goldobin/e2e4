#ifndef BOARD_H
#define BOARD_H

#include "char_slice.h"

constexpr size_t BOARD_SIDE_LEN  = 8;
constexpr size_t BOARD_SIZE      = BOARD_SIDE_LEN * BOARD_SIDE_LEN;
constexpr char   COL_CHAR_MIN    = 'a';
constexpr char   COL_CHAR_MAX    = COL_CHAR_MIN + BOARD_SIDE_LEN - 1;
constexpr char   ROW_CHAR_MIN    = '1';
constexpr char   ROW_CHAR_MAX    = ROW_CHAR_MIN + BOARD_SIDE_LEN - 1;
constexpr size_t POS_STR_LEN     = 2;
constexpr size_t MOVE_STR_LEN    = 2 * POS_STR_LEN;
constexpr size_t THREATS_CAP     = 16;
constexpr size_t PIECE_TYPES_CAP = 20;

typedef enum {
    PIECE_TYPE_UNSPECIFIED = 0,
    PIECE_TYPE_PAWN,
    PIECE_TYPE_ROOK,
    PIECE_TYPE_KNIGHT,
    PIECE_TYPE_BISHOP,
    PIECE_TYPE_QUEEN,
    PIECE_TYPE_KING
} PieceType;

typedef enum {
    SIDE_UNSPECIFIED = 0,
    SIDE_BLACK,
    SIDE_WHITE
} Side;

typedef struct {
    Side      side;
    PieceType type;
} Piece;

typedef struct {
    size_t col;
    size_t row;
} Pos;

typedef struct {
    int x;
    int y;
} Vec2I;

typedef struct {
    Pos from;
    Pos to;
} Move;

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

typedef struct {
    PieceType arr[PIECE_TYPES_CAP];
    size_t    len;
} PieceTypes;

typedef struct {
    PieceTypes taken;
    bool       hasKingCastled;
} SideState;

typedef Piece Squares[BOARD_SIDE_LEN][BOARD_SIDE_LEN];

typedef struct {
    Squares   squares;
    SideState white;
    SideState black;
    Side      nextMoveSide;
} Board;

typedef struct {
    Pos       pos;
    PieceType pieceType;
} Threat;

typedef struct {
    size_t len;
    Threat items[THREATS_CAP];
} Threats;

bool         Pos_Equals(Pos a, Pos b);
bool         Pos_IsValid(Pos a);
Side         Pos_BoardSide(Pos a);
Vec2I        Vec2I_FromPos(Pos l, Pos r);
bool         PieceType_IsValid(PieceType t);
bool         PieceTypes_Equals(const PieceTypes* a, const PieceTypes* b);
void         PieceTypes_Copy(PieceTypes* dst, const PieceTypes* src);
bool         PieceTypes_Resize(PieceTypes* dst, size_t len);
PieceType*   PieceTypes_At(PieceTypes* dst, size_t len);
bool         Piece_IsEmpty(const Piece* p);
bool         Piece_Equals(const Piece* a, const Piece* b);
bool         Move_Equals(Move a, Move b);
bool         Move_IsValid(Move a);
bool         MoveResult_Equals(const MoveResult* a, const MoveResult* b);
Piece*       Squares_At(Squares s, Pos pos);
const Piece* Squares_ConstAt(const Squares s, Pos pos);
void         Squares_Copy(Squares dst, const Squares src);
bool         Squares_Equals(const Squares a, const Squares b);
void         Squares_PlacePieces(Squares dst);
PieceType    TakenPieces_At(const PieceTypes* ts, size_t i);
bool         SideState_Equals(const SideState* a, SideState const* b);
void         SideState_Copy(const SideState* src, SideState* dst);
bool         Board_Equals(const Board* a, const Board* b);
void         Board_Copy(Board* dst, const Board* src);
MoveResult   Board_MakeMove(Board* dst, Move m);
MoveResult   Board_CheckMove(const Board* b, Move m);
MoveResult   Board_CheckPawnMove(const Board* b, Move m);
MoveResult   Board_CheckRookMove(const Board* b, Move m);
MoveResult   Board_CheckKnightMove(const Board* b, Move m);
MoveResult   Board_CheckBishopMove(const Board* b, Move m);
MoveResult   Board_CheckQueenMove(const Board* b, Move m);
MoveResult   Board_CheckKingMove(const Board* b, Move m);
bool         Threat_IsValid(Threat t);
size_t       Threats_Append(Threats* ts, Threat t);
size_t       Threats_Collect(Threats* dst, const Board* b, Pos p);

#endif  // BOARD_H
