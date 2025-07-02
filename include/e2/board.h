#ifndef E2_BOARD_H
#define E2_BOARD_H
#include <stddef.h>

constexpr size_t E2_SIDE_LEN     = 8;
constexpr size_t E2_TOTAL_CELLS  = E2_SIDE_LEN * E2_SIDE_LEN;
constexpr char   E2_COL_MIN      = 'a';
constexpr char   E2_COL_MAX      = E2_COL_MIN + E2_SIDE_LEN - 1;
constexpr char   E2_ROW_MIN      = '1';
constexpr char   E2_ROW_MAX      = E2_ROW_MIN + E2_SIDE_LEN - 1;
constexpr size_t E2_POS_STR_LEN  = 2;
constexpr size_t E2_MOVE_STR_LEN = 2 * E2_POS_STR_LEN;

typedef enum {
    E2_PIECE_TYPE_NONE = 0,
    E2_PIECE_TYPE_PAWN,
    E2_PIECE_TYPE_ROOK,
    E2_PIECE_TYPE_KNIGHT,
    E2_PIECE_TYPE_BISHOP,
    E2_PIECE_TYPE_QUEEN,
    E2_PIECE_TYPE_KING
} E2_PieceType;

char E2_PieceType_ToUpperCaseChar(E2_PieceType t);
char E2_PieceType_ToLowerCaseChar(E2_PieceType t);

typedef enum {
    E2_SIDE_NONE = 0,
    E2_SIDE_BLACK,
    E2_SIDE_WHITE
} E2_Side;

typedef struct {
    E2_Side      side;
    E2_PieceType type;
} E2_Piece;

bool E2_Piece_IsEmpty(E2_Piece p);
bool E2_Piece_Equals(E2_Piece a, E2_Piece b);
bool E2_Piece_FromChar(E2_Piece* t, char ch);

typedef struct {
    E2_Piece pieces[E2_SIDE_LEN][E2_SIDE_LEN];
} E2_Board;

typedef struct {
    size_t col;
    size_t row;
} E2_Pos;

typedef struct {
    int col;
    int row;
} E2_Diff;

typedef enum {
    E2_DIRECTION_NONE = 0,
    E2_DIRECTION_FORWARD,
    E2_DIRECTION_BACKWARD,
} E2_Direction;

E2_Direction E2_Diff_Direction(E2_Diff d, E2_Side s);

typedef enum {
    E2_POS_PARSE_OK = 0,
    E2_POS_PARSE_ERROR_TOO_SHORT,
    E2_POS_PARSE_ERROR_INVALID_FORMAT,
} E2_Pos_ParseError;

typedef struct {
    E2_Pos_ParseError err;
    size_t            offset;
} E2_Pos_ParseResult;

E2_Pos_ParseResult E2_Pos_Parse(E2_Pos* dst, const char* src);
size_t             E2_Pos_ToString(char dst[static 1], size_t max_len, E2_Pos a);
bool               E2_Pos_IsValid(E2_Pos a);

typedef struct {
    E2_Pos from;
    E2_Pos to;
} E2_Move;

typedef enum {
    E2_MOVE_PARSE_OK = 0,
    E2_MOVE_PARSE_ERROR_TOO_SHORT,
    E2_MOVE_PARSE_ERROR_INVALID_FROM_FORMAT,
    E2_MOVE_PARSE_ERROR_INVALID_TO_FORMAT,
} E2_Move_ParseError;

typedef struct {
    E2_Move_ParseError err;
    size_t             offset;
} E2_Move_ParseResult;

E2_Move_ParseResult E2_Move_Parse(E2_Move* dst, const char* src);
size_t              E2_Move_ToString(char dst[static 1], size_t max_len, E2_Move a);
bool                E2_Move_IsValid(E2_Move a);
E2_Diff             E2_Move_Diff(E2_Move a);

typedef enum {
    E2_BOARD_MOVE_OK = 0,
    E2_BOARD_MOVE_NO_PIECE,
    E2_BOARD_MOVE_ILLEGAL,
    E2_BOARD_MOVE_OBSTACLE
} E2_Board_MoveError;

typedef struct {
    E2_Board_MoveError err;
    E2_Pos             obstacleAt;
    E2_Piece           pieceTaken;
} E2_Board_MoveResult;

bool E2_Board_MoveResult_Equals(const E2_Board_MoveResult* a, const E2_Board_MoveResult* b);

typedef enum {
    E2_BOARD_PARSE_OK = 0,
    E2_BOARD_PARSE_ERROR_TOO_SHORT,
    E2_BOARD_PARSE_ERROR_UNEXPECTED_CHAR,
} E2_Board_ParseError;

typedef struct {
    E2_Board_ParseError err;
    size_t              offset;
    char                invalidChar;
} E2_Board_ParseResult;

void                 E2_Board_PlacePieces(E2_Board* b);
bool                 E2_Board_Equals(const E2_Board* a, const E2_Board* b);
E2_Board_ParseResult E2_Board_Parse(E2_Board* b, const char src[static 1]);
E2_Piece             E2_Board_At(const E2_Board* b, E2_Pos pos);
E2_Piece*            E2_Board_AtRef(E2_Board* b, E2_Pos pos);
E2_Board_MoveResult  E2_Board_Move(E2_Board* b, E2_Move m);
E2_Board_MoveResult  E2_Board_MovePawn(E2_Board* b, E2_Move m);
E2_Board_MoveResult  E2_Board_MoveRook(E2_Board* b, E2_Move m);
E2_Board_MoveResult  E2_Board_MoveKnight(E2_Board* b, E2_Move m);
E2_Board_MoveResult  E2_Board_MoveBishop(E2_Board* b, E2_Move m);
E2_Board_MoveResult  E2_Board_MoveQueen(E2_Board* b, E2_Move m);
E2_Board_MoveResult  E2_Board_MoveKing(E2_Board* b, E2_Move m);

// E2_Piece*          E2_Board_AtRef(E2_Board* b, E2_Pos pos);
bool E2_Board_IsEmpty(const E2_Board* b, E2_Pos pos);
void E2_Board_Print(const E2_Board* b);

#endif  // E2_BOARD_H
