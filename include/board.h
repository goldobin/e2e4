#ifndef BOARD_H
#define BOARD_H

#include <time.h>

#include "chars.h"

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
constexpr size_t POSITIONS_CAP   = 8;

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
    size_t row;
    size_t col;
} Pos;

typedef struct {
    Pos    arr[POSITIONS_CAP];
    size_t len;
} Positions;

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
    MOVE_ERR_ILLEGAL_MOVE,
    MOVE_ERR_OBSTACLE
} MoveErr;

typedef struct {
    MoveErr   err;
    Pos       obstacleAt;
    PieceType taken;
} MoveResult;

typedef struct {
    PieceType arr[PIECE_TYPES_CAP];
    size_t    len;
} PieceTypes;

typedef struct {
    PieceTypes taken;
    bool       check;
    bool       hasKingCastled;
} SideState;

typedef Piece Squares[BOARD_SIDE_LEN][BOARD_SIDE_LEN];

constexpr size_t MOVE_SLICE_GROW_FACTOR = 2;

typedef struct {
    Move   move;
    Piece  pice;
    time_t time;
} Step;

typedef struct {
    Step*  arr;
    size_t len;
    size_t cap;
    Arena* a;
} Steps;

typedef enum {
    BOARD_STATE_IN_UNSPECIFIED = 0,
    BOARD_STATE_IN_PROGRESS,
    BOARD_STATE_CHECKMATE,
    BOARD_STATE_STALEMATE,
} BoardState;

typedef struct {
    Squares    squares;
    BoardState state;
    Side       side;
    SideState  white;
    SideState  black;
    Steps      steps;
} Board;

typedef struct {
    Pos       pos;
    PieceType pieceType;
} Threat;

typedef struct {
    size_t len;
    Threat items[THREATS_CAP];
} Threats;

bool             Pos_Equals(Pos a, Pos b);
bool             Pos_IsValid(Pos a);
Side             Pos_BoardSide(Pos a);
bool             Pos_Find(Pos* dst, const Squares src, Piece p);
void             Positions_Around(Positions* dst, Pos p);
void             Positions_Append(Positions* dst, Pos p);
Side             Side_Opposite(Side s);
bool             PieceType_IsValid(PieceType t);
bool             PieceTypes_Equals(PieceTypes a, PieceTypes b);
bool             PieceTypes_Resize(PieceTypes* dst, size_t len);
void             PieceTypes_Push(PieceTypes* dst, PieceType t);
void             PieceTypes_UpdateAt(PieceTypes* dst, size_t i, PieceType t);
PieceType        PieceTypes_At(PieceTypes ts, size_t i);
bool             Piece_IsEmpty(Piece p);
bool             Piece_Equals(Piece a, Piece b);
bool             Move_Equals(Move a, Move b);
bool             Move_IsValid(Move a);
bool             MoveResult_Equals(const MoveResult* a, const MoveResult* b);
Piece            Squares_At(const Squares ss, Pos pos);
void             Squares_UpdateAt(Squares ss, Pos pos, Piece p);
void             Squares_Copy(Squares dst, const Squares src);
bool             Squares_Equals(const Squares ss, const Squares b);
void             Squares_InitDefaultLayout(Squares dst);
bool             Squares_IsThreatened(const Squares ss, Pos pos);
bool             Squares_CanKingMove(const Squares ss, Pos p);
void             Squares_Move(Squares dst, Pos to, Pos from);
MoveResult       Squares_CheckMove(const Squares ss, Move m);
MoveResult       Squares_CheckPawnMove(const Squares ss, Move m);
MoveResult       Squares_CheckRookMove(const Squares ss, Move m);
MoveResult       Squares_CheckKnightMove(const Squares ss, Move m);
MoveResult       Squares_CheckBishopMove(const Squares ss, Move m);
MoveResult       Squares_CheckQueenMove(const Squares ss, Move m);
MoveResult       Squares_CheckKingMove(const Squares ss, Move m);
bool             SideState_Equals(const SideState* a, SideState const* b);
void             SideState_Copy(const SideState* src, SideState* dst);
void             Board_Init(Board* dst, Steps steps);
void             Board_StartNewGame(Board* dst);
bool             Board_Equals(const Board* a, const Board* b);
void             Board_Copy(Board* dst, const Board* src);
SideState*       Board_SideStateRef(Board* b, Side s);
const SideState* Board_SideState(const Board* b, Side s);
MoveResult       Board_MakeMove(Board* dst, Move m);
bool             Threat_IsValid(Threat t);
void             Threats_Append(Threats* dst, Threat t);
void             Threats_Collect(Threats* dst, const Squares ss, Pos p);
bool             Step_Equals(const Step* a, const Step* b);
Steps            Steps_Slice(const Steps* s, size_t start, size_t end);
Step*            Steps_AtRef(Steps* s, size_t i);
const Step*      Steps_At(const Steps* s, size_t i);
Step*            Steps_Append(Steps* dst, size_t len);
bool             Steps_Equals(Steps a, Steps b);
Steps            Arena_AllocSteps(Arena* a, size_t len, size_t cap, bool autoGrow);

#define Steps_OnStack(len1, cap1) ((Steps){.arr = (Step[(cap1)]){}, .len = (len1), .cap = (cap1)})
#endif  // BOARD_H
