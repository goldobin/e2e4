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

bool PieceTypes_Equals(const PieceTypes a, const PieceTypes b) {
    if (a.len != b.len) {
        return false;
    }
    for (size_t i = 0; i < a.len; i++) {
        if (a.arr[i] != b.arr[i]) {
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

bool Piece_Equals(const Piece a, const Piece b) { return a.type == b.type && a.side == b.side; }

Side Pos_BoardSide(const Pos a) {
    assert(Pos_IsValid(a));
    return a.row < BOARD_SIDE_LEN / 2 ? SIDE_BLACK : SIDE_WHITE;
}

static Vec2I Vec2I_FromPos(const Pos l, const Pos r) {
    return (Vec2I){
        .x = (int)r.col - (int)l.col,
        .y = (int)r.row - (int)l.row,
    };
}

bool Piece_IsEmpty(Piece p) { return p.type == PIECE_TYPE_UNSPECIFIED; }

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
void PieceTypes_Push(PieceTypes* dst, PieceType t) {
    assert(dst != nullptr);
    assert(dst->len < PIECE_TYPES_CAP);

    dst->arr[dst->len++] = t;
}

PieceType PieceTypes_At(PieceTypes slice, size_t i) {
    assert(i < slice.len);
    return slice.arr[i];
}

void PieceTypes_UpdateAt(PieceTypes* dst, size_t i, PieceType t) {
    assert(dst != nullptr);
    assert(i < dst->len);

    dst->arr[i] = t;
}

bool Move_Equals(const Move a, const Move b) { return Pos_Equals(a.from, b.from) && Pos_Equals(a.to, b.to); }

bool Move_IsValid(const Move a) {
    return Pos_IsValid(a.from) && Pos_IsValid(a.to) && Pos_Equals(a.from, a.to) == false;
}

bool MoveResult_Equals(const MoveResult* a, const MoveResult* b) {
    assert(a != nullptr);
    assert(b != nullptr);

    return a->err == b->err && Piece_Equals(a->pieceTaken, b->pieceTaken) && Pos_Equals(a->obstacleAt, b->obstacleAt);
}

void Threats_Append(Threats* dst, const Threat t) {
    assert(dst != nullptr);
    assert(Threat_IsValid(t));
    assert(dst->len < THREATS_CAP);

    dst->items[dst->len++] = t;
}

void Squares_InitDefaultLayout(Squares dst) {
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

bool Pos_Find(Pos* dst, const Squares src, const Piece p) {
    assert(src != nullptr);
    assert(dst != nullptr);

    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            if (Piece_Equals(src[i][j], p)) {
                *dst = (Pos){.row = i, .col = j};
                return true;
            }
        }
    }
    return false;
}

void Positions_Around(Positions* dst, const Pos p) {
    assert(dst != nullptr);
    assert(Pos_IsValid(p));

    constexpr size_t cap     = 8;
    const Vec2I      vs[cap] = {
        {.x = (int)p.col - 1, .y = (int)p.row + 1}, {.x = (int)p.col - 1, .y = (int)p.row},
        {.x = (int)p.col - 1, .y = (int)p.row - 1}, {.x = (int)p.col, .y = (int)p.row - 1},
        {.x = (int)p.col + 1, .y = (int)p.row - 1}, {.x = (int)p.col + 1, .y = (int)p.row},
        {.x = (int)p.col + 1, .y = (int)p.row + 1}, {.x = (int)p.col, .y = (int)p.row + 1},
    };

    for (size_t i = 0; i < cap; i++) {
        const auto v = vs[i];
        if (v.x < 0 || v.x >= (int)BOARD_SIDE_LEN || v.y < 0 || v.y >= (int)BOARD_SIDE_LEN) {
            continue;
        }

        const auto pos = (Pos){.col = v.x, .row = v.y};
        Positions_Append(dst, pos);
    }
}

void Positions_Append(Positions* dst, Pos p) {
    assert(dst != nullptr);
    assert(Pos_IsValid(p));
    assert(dst->len < POSITIONS_CAP);

    dst->arr[dst->len++] = p;
}
Side Side_Opposite(const Side s) {
    switch (s) {
        case SIDE_WHITE:
            return SIDE_BLACK;
        case SIDE_BLACK:
            return SIDE_WHITE;
        default:
            assert(false);
    }
}

bool Squares_IsThreatened(const Squares ss, Pos pos) {
    assert(ss != nullptr);
    Threats ts = {};
    Threats_Collect(&ts, ss, pos);
    return ts.len > 0;
}

bool Squares_CanKingMove(const Squares ss, const Pos p) {
    assert(ss != nullptr);
    assert(Pos_IsValid(p));

    const auto piece = Squares_At(ss, p);
    assert(piece.type == PIECE_TYPE_KING);

    Positions ps = {};
    Positions_Around(&ps, p);
    for (size_t i = 0; i < ps.len; i++) {
        const auto m = (Move){.from = p, .to = ps.arr[i]};
        const auto r = Squares_CheckKingMove(ss, m);
        if (r.err != MOVE_ERR_OK) {
            continue;
        }

        Squares ssCopy = {};
        Squares_Copy(ssCopy, ss);
        Squares_Move(ssCopy, m.to, m.from);
        if (!Squares_IsThreatened(ssCopy, m.to)) {
            return true;
        }
    }
    return false;
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

bool Squares_Equals(const Squares ss, const Squares b) {
    assert(ss != nullptr);
    assert(b != nullptr);

    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            if (!Piece_Equals(ss[i][j], b[i][j])) {
                return false;
            }
        }
    }
    return true;
}

Piece Squares_At(const Squares ss, const Pos pos) {
    assert(ss != nullptr);
    assert(Pos_IsValid(pos));

    return ss[pos.row][pos.col];
}
void Squares_UpdateAt(Squares ss, const Pos pos, const Piece p) {
    assert(ss != nullptr);
    assert(Pos_IsValid(pos));

    ss[pos.row][pos.col] = p;
}

void Squares_Move(Squares dst, const Pos to, const Pos from) {
    const auto piece = Squares_At(dst, from);
    Squares_UpdateAt(dst, from, (Piece){});
    Squares_UpdateAt(dst, to, piece);
}

bool SideState_Equals(const SideState* a, SideState const* b) {
    assert(a != nullptr);
    assert(b != nullptr);

    if (a->hasKingCastled != b->hasKingCastled) {
        return false;
    }
    if (a->check != b->check) {
        return false;
    }
    if (!PieceTypes_Equals(a->taken, b->taken)) {
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
void Board_Init(Board* dst) {
    assert(dst != nullptr);

    dst->side  = SIDE_WHITE;
    dst->state = BOARD_STATE_IN_PROGRESS;
    Squares_InitDefaultLayout(dst->squares);
}

bool Board_Equals(const Board* a, const Board* b) {
    assert(a != nullptr);
    assert(b != nullptr);

    if (a->state != b->state) {
        return false;
    }
    if (a->side != b->side) {
        return false;
    }
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

    dst->side = src->side;
    SideState_Copy(&src->white, &dst->white);
    SideState_Copy(&src->black, &dst->black);
    Squares_Copy(dst->squares, src->squares);
}
SideState* Board_SideStateRef(Board* b, const Side s) {
    switch (s) {
        case SIDE_WHITE:
            return &b->white;
        case SIDE_BLACK:
            return &b->black;
        default:
            assert(false);
    }
}

MoveResult Board_MakeMove(Board* dst, const Move m) {
    assert(dst != nullptr);
    assert(Move_IsValid(m));
    assert(dst->state == BOARD_STATE_IN_PROGRESS);
    assert(dst->side != SIDE_UNSPECIFIED);

    const auto result = Squares_CheckMove(dst->squares, m);
    if (result.err != MOVE_ERR_OK) {
        return result;
    }

    const auto piece = Squares_At(dst->squares, m.from);
    if (piece.type == PIECE_TYPE_KING) {
        Squares squaresCopy = {};
        Squares_Copy(squaresCopy, dst->squares);
        Squares_Move(squaresCopy, m.to, m.from);
        if (Squares_IsThreatened(squaresCopy, m.to)) {
            return (MoveResult){.err = MOVE_ERR_ILLEGAL_MOVE};
        }
    }

    const auto side              = dst->side;
    const auto oppositeSide      = Side_Opposite(dst->side);
    const auto sideState         = Board_SideStateRef(dst, side);
    const auto oppositeSideState = Board_SideStateRef(dst, Side_Opposite(side));

    Squares_Move(dst->squares, m.to, m.from);
    if (result.pieceTaken.type != PIECE_TYPE_UNSPECIFIED) {
        PieceTypes_Push(&sideState->taken, result.pieceTaken.type);
    }

    const Piece kingPiece = {.side = side, .type = PIECE_TYPE_KING};
    Pos         kingPos   = {};
    if (!Pos_Find(&kingPos, dst->squares, kingPiece)) {
        assert(false);
    };

    const Piece oppositeKingPiece = {.side = oppositeSide, .type = PIECE_TYPE_KING};
    Pos         otherKingPos      = {};
    if (!Pos_Find(&otherKingPos, dst->squares, oppositeKingPiece)) {
        assert(false);
    };

    sideState->check         = Squares_IsThreatened(dst->squares, kingPos);
    oppositeSideState->check = Squares_IsThreatened(dst->squares, otherKingPos);
    if (oppositeSideState->check && !Squares_CanKingMove(dst->squares, otherKingPos)) {
        dst->state = BOARD_STATE_CHECKMATE;
        return result;
    }

    dst->side = oppositeSide;
    return result;
}

MoveResult Squares_CheckMove(const Squares ss, const Move m) {
    assert(ss != nullptr);
    assert(Move_IsValid(m));

    if (Pos_Equals(m.from, m.to)) {
        return (MoveResult){.err = MOVE_ERR_NO_MOVEMENT};
    }

    const auto piece = Squares_At(ss, m.from);
    switch (piece.type) {
        case PIECE_TYPE_UNSPECIFIED:
        default:
            return (MoveResult){.err = MOVE_ERR_NO_PIECE};
        case PIECE_TYPE_PAWN:
            return Squares_CheckPawnMove(ss, m);
        case PIECE_TYPE_ROOK:
            return Squares_CheckRookMove(ss, m);
        case PIECE_TYPE_KNIGHT:
            return Squares_CheckKnightMove(ss, m);
        case PIECE_TYPE_BISHOP:
            return Squares_CheckBishopMove(ss, m);
        case PIECE_TYPE_QUEEN:
            return Squares_CheckQueenMove(ss, m);
        case PIECE_TYPE_KING:
            return Squares_CheckKingMove(ss, m);
    }
}

int sign(const int a) { return a > 0 ? 1 : a < 0 ? -1 : 0; }

MoveResult Squares_CheckPawnMove(const Squares ss, const Move m) {
    assert(ss != nullptr);
    assert(Move_IsValid(m));
    const auto direction      = Vec2I_FromPos(m.from, m.to);
    const auto piece          = Squares_At(ss, m.from);
    const auto dstPiece       = Squares_At(ss, m.to);
    const auto validDirection = piece.side == SIDE_WHITE ? -1 : 1;

    if (sign(direction.y) != validDirection) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL_MOVE};
    }

    const auto distance = abs(direction.y);

    if (dstPiece.side != piece.side && distance && abs(direction.x) == 1) {
        return (MoveResult){.err = MOVE_ERR_OK, .pieceTaken = dstPiece};
    }

    if (direction.x != 0) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL_MOVE};
    }

    if (Pos_BoardSide(m.to) != piece.side && distance > 1) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL_MOVE};
    }

    for (int i = 1; i < distance + 1; i++) {
        const Pos p = {
            .row = m.from.row + sign(direction.y) * i,
            .col = m.from.col,
        };
        const auto obstacle = Squares_At(ss, p);
        if (Piece_IsEmpty(obstacle)) {
            continue;
        }

        return (MoveResult){
            .err        = MOVE_ERR_OBSTACLE,
            .obstacleAt = p,
        };
    }

    return (MoveResult){.err = MOVE_ERR_OK};
}

MoveResult Squares_CheckRookMove(const Squares ss, const Move m) {
    assert(ss != nullptr);
    assert(Move_IsValid(m));

    const auto direction = Vec2I_FromPos(m.from, m.to);
    const auto piece     = Squares_At(ss, m.from);

    if (direction.y != 0 && direction.x != 0) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL_MOVE};
    }

    const auto distance   = MaxSizeT((size_t)abs(direction.y), (size_t)abs(direction.x));
    Piece      pieceTaken = {};
    for (size_t i = 1; i < distance + 1; i++) {
        const Pos p = {
            .row = m.from.row + sign(direction.y) * i,
            .col = m.from.col + sign(direction.x) * i,
        };
        const auto square = Squares_At(ss, p);
        if (Piece_IsEmpty(square)) {
            continue;
        }

        if (i == distance && square.side != piece.side) {
            pieceTaken = square;
            break;
        }

        return (MoveResult){
            .err        = MOVE_ERR_OBSTACLE,
            .obstacleAt = p,
        };
    }

    return (MoveResult){
        .err        = MOVE_ERR_OK,
        .pieceTaken = pieceTaken,
    };
}

MoveResult Squares_CheckKnightMove(const Squares ss, const Move m) {
    assert(ss != nullptr);
    assert(Move_IsValid(m));

    const auto direction = Vec2I_FromPos(m.from, m.to);
    const auto piece     = Squares_At(ss, m.from);
    const auto dstPiece  = Squares_At(ss, m.to);

    if ((abs(direction.y) != 2 || abs(direction.x) != 1) && (abs(direction.y) != 1 || abs(direction.x) != 2)) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL_MOVE};
    }

    Piece pieceTaken = {};
    if (!Piece_IsEmpty(dstPiece)) {
        if (dstPiece.side == piece.side) {
            return (MoveResult){.err = MOVE_ERR_OBSTACLE, .obstacleAt = m.to};
        }

        pieceTaken = dstPiece;
    }

    return (MoveResult){
        .err        = MOVE_ERR_OK,
        .pieceTaken = pieceTaken,
    };
}

MoveResult Squares_CheckBishopMove(const Squares ss, const Move m) {
    assert(ss != nullptr);
    assert(Move_IsValid(m));

    const auto direction = Vec2I_FromPos(m.from, m.to);
    const auto piece     = Squares_At(ss, m.from);

    if (abs(direction.y) != abs(direction.x)) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL_MOVE};
    }

    const auto distance   = (size_t)abs(direction.y);
    Piece      pieceTaken = {};
    for (size_t i = 1; i < distance + 1; i++) {
        const Pos p = {
            .row = m.from.row + sign(direction.y) * i,
            .col = m.from.col + sign(direction.x) * i,
        };
        const auto square = Squares_At(ss, p);
        if (Piece_IsEmpty(square)) {
            continue;
        }

        if (i == distance && square.side != piece.side) {
            pieceTaken = square;
            break;
        }

        return (MoveResult){
            .err        = MOVE_ERR_OBSTACLE,
            .obstacleAt = p,
        };
    }

    return (MoveResult){
        .err        = MOVE_ERR_OK,
        .pieceTaken = pieceTaken,
    };
}

MoveResult Squares_CheckQueenMove(const Squares ss, const Move m) {
    assert(ss);
    assert(Move_IsValid(m));

    const auto moveRookResult = Squares_CheckRookMove(ss, m);
    switch (moveRookResult.err) {
        case MOVE_ERR_OK:
        case MOVE_ERR_OBSTACLE:
            return moveRookResult;
        default:
            break;
    }

    const auto moveBishopResult = Squares_CheckBishopMove(ss, m);
    switch (moveBishopResult.err) {
        case MOVE_ERR_OK:
        case MOVE_ERR_OBSTACLE:
            return moveBishopResult;
        default:
            break;
    }
    return (MoveResult){
        .err = MOVE_ERR_ILLEGAL_MOVE,
    };
}

MoveResult Squares_CheckKingMove(const Squares ss, const Move m) {
    assert(ss != nullptr);
    assert(Move_IsValid(m));

    const auto direction = Vec2I_FromPos(m.from, m.to);
    const auto piece     = Squares_At(ss, m.from);
    const auto dstPiece  = Squares_At(ss, m.to);

    if (abs(direction.y) > 1 || abs(direction.x) > 1) {
        return (MoveResult){.err = MOVE_ERR_ILLEGAL_MOVE};
    }

    if (dstPiece.side == piece.side) {
        return (MoveResult){
            .err        = MOVE_ERR_OBSTACLE,
            .obstacleAt = m.to,
        };
    }

    return (MoveResult){
        .err        = MOVE_ERR_OK,
        .pieceTaken = dstPiece,
    };
}

bool Threat_IsValid(const Threat t) { return Pos_IsValid(t.pos) && PieceType_IsValid(t.pieceType); }

void Threats_Collect(Threats* dst, const Squares ss, const Pos p) {
    const auto piece = Squares_At(ss, p);
    if (Piece_IsEmpty(piece)) {
        return;
    }

    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            const Pos  opponentPos = {.col = i, .row = j};
            const auto opponent    = Squares_At(ss, opponentPos);
            if (opponent.side == piece.side) {
                continue;
            }

            const auto result = Squares_CheckMove(ss, (Move){.from = opponentPos, .to = p});
            if (result.err == MOVE_ERR_OK) {
                const Threat threat = {
                    .pos       = opponentPos,
                    .pieceType = opponent.type,
                };
                Threats_Append(dst, threat);
            }
        }
    }
}
