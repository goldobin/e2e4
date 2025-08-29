#include "board.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "func.h"
#include "json.h"

bool Pos_IsValid(const Pos a) { return a.row < BOARD_SIDE_LEN && a.col < BOARD_SIDE_LEN; }

bool Pos_Equals(const Pos a, const Pos b) { return a.row == b.row && a.col == b.col; }

bool PieceType_IsValid(const PieceType t) {
    switch (t) {
        case PIECE_TYPE_UNSPECIFIED:
        case PIECE_TYPE_PAWN:
        case PIECE_TYPE_ROOK:
        case PIECE_TYPE_KNIGHT:
        case PIECE_TYPE_BISHOP:
        case PIECE_TYPE_QUEEN:
        case PIECE_TYPE_KING:
            return true;
        default:
            return false;
    }
}

bool PieceTypes_Equals(const PieceTypes* a, const PieceTypes* b) {
    assert(a != nullptr);
    assert(b != nullptr);

    if (a->len != b->len) {
        return false;
    }
    for (size_t i = 0; i < a->len; i++) {
        if (a->arr[i] != b->arr[i]) {
            return false;
        }
    }

    return true;
}

void PieceTypes_Copy(PieceTypes* dst, const PieceTypes* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    dst->len = src->len;
    for (size_t i = 0; i < src->len; i++) {
        dst->arr[i] = src->arr[i];
    }
}

bool Piece_Equals(const Piece* a, const Piece* b) { return a->type == b->type && a->side == b->side; }

Side Pos_BoardSide(const Pos a) {
    assert(Pos_IsValid(a));
    return a.row < BOARD_SIDE_LEN / 2 ? SIDE_BLACK : SIDE_WHITE;
}

Vec2I Vec2I_FromPos(const Pos l, const Pos r) {
    return (Vec2I){
        .x = (int)r.col - (int)l.col,
        .y = (int)r.row - (int)l.row,
    };
}

bool Piece_IsEmpty(const Piece* p) { return p->type == PIECE_TYPE_UNSPECIFIED; }

bool PieceTypes_Resize(PieceTypes* dst, const size_t len) {
    assert(dst != nullptr);
    if (len == dst->len) {
        return true;
    }
    if (len > PIECE_TYPES_CAP) {
        return false;
    }

    if (len < dst->len) {
        memset(&dst->arr[len], 0, (dst->len - len) * sizeof(PieceType));
    }

    dst->len = len;
    return true;
}

PieceType* PieceTypes_At(PieceTypes* dst, size_t i) {
    assert(dst != nullptr);
    assert(i < dst->len);

    return &dst->arr[i];
}

PieceType TakenPieces_At(const PieceTypes* ts, const size_t i) {
    assert(ts != nullptr);
    assert(i < ts->len);
    return ts->arr[i];
}

bool Move_Equals(const Move a, const Move b) { return Pos_Equals(a.from, b.from) && Pos_Equals(a.to, b.to); }

bool Move_IsValid(const Move a) {
    return Pos_IsValid(a.from) && Pos_IsValid(a.to) && Pos_Equals(a.from, a.to) == false;
}

bool MoveResult_Equals(const MoveResult* a, const MoveResult* b) {
    assert(a != nullptr);
    assert(b != nullptr);
    return a->err == b->err && Piece_Equals(&a->pieceTaken, &b->pieceTaken) && Pos_Equals(a->obstacleAt, b->obstacleAt);
}

size_t Threats_Append(Threats* ts, const Threat t) {
    assert(ts != nullptr);
    assert(Threat_IsValid(t));

    if (ts->len >= THREATS_CAP) {
        return 0;
    }

    ts->items[ts->len++] = t;
    return 1;
}

void Squares_PlacePieces(Squares dst) {
    assert(dst != nullptr);

    dst[7][0] = (Piece){.type = PIECE_TYPE_ROOK, .side = SIDE_WHITE};
    dst[7][7] = (Piece){.type = PIECE_TYPE_ROOK, .side = SIDE_WHITE};
    dst[7][1] = (Piece){.type = PIECE_TYPE_KNIGHT, .side = SIDE_WHITE};
    dst[7][6] = (Piece){.type = PIECE_TYPE_KNIGHT, .side = SIDE_WHITE};
    dst[7][2] = (Piece){.type = PIECE_TYPE_BISHOP, .side = SIDE_WHITE};
    dst[7][5] = (Piece){.type = PIECE_TYPE_BISHOP, .side = SIDE_WHITE};
    dst[7][3] = (Piece){.type = PIECE_TYPE_QUEEN, .side = SIDE_WHITE};
    dst[7][4] = (Piece){.type = PIECE_TYPE_KING, .side = SIDE_WHITE};

    dst[0][0] = (Piece){.type = PIECE_TYPE_ROOK, .side = SIDE_BLACK};
    dst[0][7] = (Piece){.type = PIECE_TYPE_ROOK, .side = SIDE_BLACK};
    dst[0][1] = (Piece){.type = PIECE_TYPE_KNIGHT, .side = SIDE_BLACK};
    dst[0][6] = (Piece){.type = PIECE_TYPE_KNIGHT, .side = SIDE_BLACK};
    dst[0][2] = (Piece){.type = PIECE_TYPE_BISHOP, .side = SIDE_BLACK};
    dst[0][5] = (Piece){.type = PIECE_TYPE_BISHOP, .side = SIDE_BLACK};
    dst[0][3] = (Piece){.type = PIECE_TYPE_QUEEN, .side = SIDE_BLACK};
    dst[0][4] = (Piece){.type = PIECE_TYPE_KING, .side = SIDE_BLACK};

    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        dst[6][i] = (Piece){.type = PIECE_TYPE_PAWN, .side = SIDE_WHITE};
        dst[1][i] = (Piece){.type = PIECE_TYPE_PAWN, .side = SIDE_BLACK};
    }

    for (size_t i = 2; i < 6; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            dst[i][j].type = PIECE_TYPE_UNSPECIFIED;
            dst[i][j].side = SIDE_UNSPECIFIED;
        }
    }
}

void Squares_Copy(Squares dst, const Squares src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            dst[i][j] = src[i][j];
        }
    }
}

bool Squares_Equals(const Squares a, const Squares b) {
    assert(a != nullptr);
    assert(b != nullptr);

    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            if (!Piece_Equals(&a[i][j], &b[i][j])) {
                return false;
            }
        }
    }
    return true;
}

Piece* Squares_At(Squares s, const Pos pos) {
    assert(s != nullptr);
    assert(Pos_IsValid(pos));
    return &s[pos.row][pos.col];
}
const Piece* Squares_ConstAt(const Squares s, Pos pos) {
    assert(s != nullptr);
    assert(Pos_IsValid(pos));
    return &s[pos.row][pos.col];
}

bool SideState_Equals(const SideState* a, SideState const* b) {
    assert(a != nullptr);
    assert(b != nullptr);

    if (a->hasKingCastled != b->hasKingCastled) {
        return false;
    }

    if (!PieceTypes_Equals(&a->taken, &b->taken)) {
        return false;
    }

    return true;
}

void SideState_Copy(const SideState* src, SideState* dst) {
    assert(dst != nullptr);
    assert(src != nullptr);

    dst->hasKingCastled = src->hasKingCastled;
    PieceTypes_Copy(&dst->taken, &src->taken);
}

bool Board_Equals(const Board* a, const Board* b) {
    assert(a != nullptr);
    assert(b != nullptr);

    if (a->nextMoveSide != b->nextMoveSide) {
        return false;
    };
    if (!SideState_Equals(&a->white, &b->white)) {
        return false;
    }
    if (!SideState_Equals(&a->black, &b->black)) {
        return false;
    }
    if (!Squares_Equals(a->squares, b->squares)) {
        return false;
    }

    return true;
}

void Board_Copy(Board* dst, const Board* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    dst->nextMoveSide = src->nextMoveSide;
    SideState_Copy(&src->white, &dst->white);
    SideState_Copy(&src->black, &dst->black);
    Squares_Copy(dst->squares, src->squares);
}

MoveResult Board_MakeMove(Board* dst, const Move m) {
    const auto result = Board_CheckMove(dst, m);
    if (result.err != MOVE_ERR_OK) {
        return result;
    }

    const auto piece = *Squares_At(dst->squares, m.from);
    if (piece.type == PIECE_TYPE_KING) {
        Board   dstCopy = {};
        Threats ts      = {};
        Board_Copy(&dstCopy, dst);
        const auto fromSquare = Squares_At(dstCopy.squares, m.from);
        const auto toSquare   = Squares_At(dstCopy.squares, m.to);
        *fromSquare           = (Piece){};
        *toSquare             = piece;

        Threats_Collect(&ts, &dstCopy, m.to);
        if (ts.len > 0) {
            return (MoveResult){.err = MOVE_ERR_ILLEGAL};
        }
        return result;
    }

    const auto fromSquare = Squares_At(dst->squares, m.from);
    const auto toSquare   = Squares_At(dst->squares, m.to);
    *fromSquare           = (Piece){};
    *toSquare             = piece;
    return result;
}

MoveResult Board_CheckMove(const Board* b, const Move m) {
    assert(b != nullptr);
    assert(Move_IsValid(m));

    if (Pos_Equals(m.from, m.to)) {
        return (MoveResult){.err = MOVE_ERR_NO_MOVEMENT};
    }

    const auto piece = Squares_ConstAt(b->squares, m.from);
    switch (piece->type) {
        case PIECE_TYPE_UNSPECIFIED:
        default:
            return (MoveResult){.err = MOVE_ERR_NO_PIECE};
        case PIECE_TYPE_PAWN:
            return Board_CheckPawnMove(b, m);
        case PIECE_TYPE_ROOK:
            return Board_CheckRookMove(b, m);
        case PIECE_TYPE_KNIGHT:
            return Board_CheckKnightMove(b, m);
        case PIECE_TYPE_BISHOP:
            return Board_CheckBishopMove(b, m);
        case PIECE_TYPE_QUEEN:
            return Board_CheckQueenMove(b, m);
        case PIECE_TYPE_KING:
            return Board_CheckKingMove(b, m);
    }
}

int sign(const int a) { return a > 0 ? 1 : a < 0 ? -1 : 0; }

MoveResult Board_CheckPawnMove(const Board* b, const Move m) {
    assert(b != nullptr);
    assert(Move_IsValid(m));
    const auto direction      = Vec2I_FromPos(m.from, m.to);
    const auto piece          = Squares_ConstAt(b->squares, m.from);
    const auto dstPiece       = Squares_ConstAt(b->squares, m.to);
    const auto validDirection = piece->side == SIDE_WHITE ? -1 : 1;

    if (sign(direction.y) != validDirection) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL};
    }

    const auto distance = abs(direction.y);

    if (dstPiece->side != piece->side && distance && abs(direction.x) == 1) {
        return (MoveResult){.err = MOVE_ERR_OK, .pieceTaken = *dstPiece};
    }

    if (direction.x != 0) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL};
    }

    if (Pos_BoardSide(m.to) != piece->side && distance > 1) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL};
    }

    for (int i = 1; i < distance + 1; i++) {
        const Pos  p        = {.row = m.from.row + sign(direction.y) * i, .col = m.from.col};
        const auto obstacle = Squares_ConstAt(b->squares, p);
        if (Piece_IsEmpty(obstacle)) {
            continue;
        }

        return (MoveResult){.err = MOVE_ERR_OBSTACLE, .obstacleAt = p};
    }

    return (MoveResult){.err = MOVE_ERR_OK};
}

MoveResult Board_CheckRookMove(const Board* b, const Move m) {
    assert(b != nullptr);
    assert(Move_IsValid(m));

    const auto direction = Vec2I_FromPos(m.from, m.to);
    const auto piece     = Squares_ConstAt(b->squares, m.from);

    if (direction.y != 0 && direction.x != 0) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL};
    }

    const auto distance   = MaxSizeT((size_t)abs(direction.y), (size_t)abs(direction.x));
    Piece      pieceTaken = {};
    for (size_t i = 1; i < distance + 1; i++) {
        const Pos  p      = {.row = m.from.row + sign(direction.y) * i, .col = m.from.col + sign(direction.x) * i};
        const auto square = Squares_ConstAt(b->squares, p);
        if (Piece_IsEmpty(square)) {
            continue;
        }

        if (i == distance && square->side != piece->side) {
            pieceTaken = *square;
            break;
        }

        return (MoveResult){.err = MOVE_ERR_OBSTACLE, .obstacleAt = p};
    }

    return (MoveResult){.err = MOVE_ERR_OK, .pieceTaken = pieceTaken};
}

MoveResult Board_CheckKnightMove(const Board* b, const Move m) {
    assert(b != nullptr);
    assert(Move_IsValid(m));

    const auto direction = Vec2I_FromPos(m.from, m.to);
    const auto piece     = Squares_ConstAt(b->squares, m.from);
    const auto dstPiece  = Squares_ConstAt(b->squares, m.to);

    if ((abs(direction.y) != 2 || abs(direction.x) != 1) && (abs(direction.y) != 1 || abs(direction.x) != 2)) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL};
    }

    Piece pieceTaken = {};
    if (!Piece_IsEmpty(dstPiece)) {
        if (dstPiece->side == piece->side) {
            return (MoveResult){.err = MOVE_ERR_OBSTACLE, .obstacleAt = m.to};
        }

        pieceTaken = *dstPiece;
    }

    return (MoveResult){.err = MOVE_ERR_OK, .pieceTaken = pieceTaken};
}

MoveResult Board_CheckBishopMove(const Board* b, const Move m) {
    assert(b != nullptr);
    assert(Move_IsValid(m));

    const auto direction = Vec2I_FromPos(m.from, m.to);
    const auto piece     = Squares_ConstAt(b->squares, m.from);

    if (abs(direction.y) != abs(direction.x)) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL};
    }

    const auto distance   = (size_t)abs(direction.y);
    Piece      pieceTaken = {};
    for (size_t i = 1; i < distance + 1; i++) {
        const Pos  p      = {.row = m.from.row + sign(direction.y) * i, .col = m.from.col + sign(direction.x) * i};
        const auto square = Squares_ConstAt(b->squares, p);
        if (Piece_IsEmpty(square)) {
            continue;
        }

        if (i == distance && square->side != piece->side) {
            pieceTaken = *square;
            break;
        }

        return (MoveResult){.err = MOVE_ERR_OBSTACLE, .obstacleAt = p};
    }

    return (MoveResult){.err = MOVE_ERR_OK, .pieceTaken = pieceTaken};
}

MoveResult Board_CheckQueenMove(const Board* b, const Move m) {
    assert(b != nullptr);
    assert(Move_IsValid(m));

    const auto moveRookResult = Board_CheckRookMove(b, m);
    switch (moveRookResult.err) {
        case MOVE_ERR_OK:
        case MOVE_ERR_OBSTACLE:
            return moveRookResult;
        default:
            break;
    }

    const auto moveBishopResult = Board_CheckBishopMove(b, m);
    switch (moveBishopResult.err) {
        case MOVE_ERR_OK:
        case MOVE_ERR_OBSTACLE:
            return moveBishopResult;
        default:
            break;
    }
    return (MoveResult){
        .err = MOVE_ERR_ILLEGAL,
    };
}

MoveResult Board_CheckKingMove(const Board* b, const Move m) {
    assert(b != nullptr);
    assert(Move_IsValid(m));

    const auto direction = Vec2I_FromPos(m.from, m.to);
    const auto piece     = Squares_ConstAt(b->squares, m.from);
    const auto dstPiece  = Squares_ConstAt(b->squares, m.to);

    if (abs(direction.y) > 1 || abs(direction.x) > 1) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL};
    }

    if (dstPiece->side == piece->side) {
        return (MoveResult){.err = MOVE_ERR_OBSTACLE, .obstacleAt = m.to};
    }

    return (MoveResult){.err = MOVE_ERR_OK, .pieceTaken = *dstPiece};
}

bool Threat_IsValid(const Threat t) { return Pos_IsValid(t.pos) && PieceType_IsValid(t.pieceType); }

size_t Threats_Collect(Threats* dst, const Board* b, const Pos p) {
    const auto piece = Squares_ConstAt(b->squares, p);
    if (Piece_IsEmpty(piece)) {
        return 0;
    }

    size_t collected = 0;
    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            const Pos  opponentPos = {.col = i, .row = j};
            const auto opponent    = Squares_ConstAt(b->squares, opponentPos);
            if (opponent->side == piece->side) {
                continue;
            }

            const auto result = Board_CheckMove(b, (Move){.from = opponentPos, .to = p});
            if (result.err == MOVE_ERR_OK) {
                collected += Threats_Append(dst, (Threat){.pos = opponentPos, .pieceType = opponent->type});
            }
        }
    }

    return collected;
}
