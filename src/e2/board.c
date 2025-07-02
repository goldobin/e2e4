#include "e2/board.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "u/func.h"

char E2_PieceType_ToUpperCaseChar(const E2_PieceType t) {
    switch (t) {
        case E2_PIECE_TYPE_PAWN:
            return 'P';
        case E2_PIECE_TYPE_ROOK:
            return 'R';
        case E2_PIECE_TYPE_KNIGHT:
            return 'N';
        case E2_PIECE_TYPE_BISHOP:
            return 'B';
        case E2_PIECE_TYPE_QUEEN:
            return 'Q';
        case E2_PIECE_TYPE_KING:
            return 'K';
        default:
            return '.';
    }
}

bool E2_Piece_FromChar(E2_Piece* t, char ch) {
    switch (ch) {
        case 'P':
            t->type = E2_PIECE_TYPE_PAWN;
            t->side = E2_SIDE_WHITE;
            return true;
        case 'R':
            t->type = E2_PIECE_TYPE_ROOK;
            t->side = E2_SIDE_WHITE;
            return true;
        case 'N':
            t->type = E2_PIECE_TYPE_KNIGHT;
            t->side = E2_SIDE_WHITE;
            return true;
        case 'B':
            t->type = E2_PIECE_TYPE_BISHOP;
            t->side = E2_SIDE_WHITE;
            return true;
        case 'Q':
            t->type = E2_PIECE_TYPE_QUEEN;
            t->side = E2_SIDE_WHITE;
            return true;
        case 'K':
            t->type = E2_PIECE_TYPE_KING;
            t->side = E2_SIDE_BLACK;
            return true;
        case 'p':
            t->type = E2_PIECE_TYPE_PAWN;
            t->side = E2_SIDE_BLACK;
            return true;
        case 'r':
            t->type = E2_PIECE_TYPE_ROOK;
            t->side = E2_SIDE_BLACK;
            return true;
        case 'n':
            t->type = E2_PIECE_TYPE_KNIGHT;
            t->side = E2_SIDE_BLACK;
            return true;
        case 'b':
            t->type = E2_PIECE_TYPE_BISHOP;
            t->side = E2_SIDE_BLACK;
            return true;
        case 'q':
            t->type = E2_PIECE_TYPE_QUEEN;
            t->side = E2_SIDE_BLACK;
            return true;
        case 'k':
            t->type = E2_PIECE_TYPE_KING;
            t->side = E2_SIDE_BLACK;
            return true;
        case '.':
        case ' ':
            t->type = E2_PIECE_TYPE_NONE;
            t->side = E2_SIDE_NONE;
            return true;
        default:
            return false;
    }
}

char E2_PieceType_ToLowerCaseChar(const E2_PieceType t) {
    switch (t) {
        case E2_PIECE_TYPE_PAWN:
            return 'p';
        case E2_PIECE_TYPE_ROOK:
            return 'r';
        case E2_PIECE_TYPE_KNIGHT:
            return 'n';
        case E2_PIECE_TYPE_BISHOP:
            return 'b';
        case E2_PIECE_TYPE_QUEEN:
            return 'q';
        case E2_PIECE_TYPE_KING:
            return 'k';
        default:
            return '.';
    }
}

bool E2_PieceType_FromLowerCaseChar(E2_PieceType* t, const char ch) {
    switch (ch) {
        case '.':
        case ' ':
            *t = E2_PIECE_TYPE_NONE;
            return true;
        default:
            return false;
    }
}

bool E2_Pos_IsValid(const E2_Pos a) { return a.row < E2_SIDE_LEN && a.col < E2_SIDE_LEN; }

bool E2_Pos_Equals(const E2_Pos a, const E2_Pos b) { return a.row == b.row && a.col == b.col; }

size_t E2_Pos_ToString(char dst[static 1], size_t max_len, const E2_Pos a) {
    assert(E2_Pos_IsValid(a));
    const char col = (char)(E2_COL_MIN + a.col);
    return snprintf(dst, max_len, "%c%zd", col, a.row + 1);
}

E2_Direction E2_Diff_Direction(const E2_Diff d, const E2_Side s) {
    assert(s != E2_SIDE_NONE);
    if (d.row == 0) {
        return E2_DIRECTION_NONE;
    }

    return s == E2_SIDE_WHITE && d.row > 0 ? E2_DIRECTION_FORWARD : E2_DIRECTION_BACKWARD;
}

E2_Pos_ParseResult E2_Pos_Parse(E2_Pos* dst, const char* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    const size_t src_len = strnlen(src, E2_POS_STR_LEN + 1);
    if (src_len < E2_POS_STR_LEN) {
        return (E2_Pos_ParseResult){.err = E2_POS_PARSE_ERROR_TOO_SHORT};
    }

    if (src[0] < E2_COL_MIN || src[0] > E2_COL_MAX || src[1] < E2_ROW_MIN || src[1] > E2_ROW_MAX) {
        return (E2_Pos_ParseResult){.err = E2_POS_PARSE_ERROR_INVALID_FORMAT};
    }

    dst->col = src[0] - E2_COL_MIN;
    dst->row = E2_SIDE_LEN - (src[1] - E2_ROW_MIN) - 1;
    return (E2_Pos_ParseResult){.offset = 2};
}

E2_Move_ParseResult E2_Move_Parse(E2_Move* dst, const char* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    if (strnlen(src, E2_MOVE_STR_LEN + 1) < E2_MOVE_STR_LEN) {
        return (E2_Move_ParseResult){.err = E2_MOVE_PARSE_ERROR_TOO_SHORT};
    }

    E2_Pos     from, to;
    const auto from_parse_result = E2_Pos_Parse(&from, src);
    if (from_parse_result.err != E2_POS_PARSE_OK) {
        return (E2_Move_ParseResult){.err = E2_MOVE_PARSE_ERROR_INVALID_FROM_FORMAT};
    }

    const auto to_parse_result = E2_Pos_Parse(&to, &src[from_parse_result.offset]);
    if (to_parse_result.err != E2_POS_PARSE_OK) {
        return (E2_Move_ParseResult){.err = E2_MOVE_PARSE_ERROR_INVALID_TO_FORMAT};
    }

    dst->from = from;
    dst->to   = to;
    return (E2_Move_ParseResult){
        .offset = from_parse_result.offset + to_parse_result.offset,
    };
}

size_t E2_Move_ToString(char dst[static 1], const size_t max_len, const E2_Move a) {
    const auto from_offset = E2_Pos_ToString(dst, max_len, a.from);
    if (from_offset >= max_len) {
        return from_offset;
    }

    const auto to_offset = E2_Pos_ToString(&dst[from_offset], max_len - from_offset, a.to);
    return from_offset + to_offset;
}

bool E2_Move_Equals(const E2_Move a, const E2_Move b) {
    return E2_Pos_Equals(a.from, b.from) && E2_Pos_Equals(a.to, b.to);
}

bool E2_Move_IsValid(const E2_Move a) {
    return E2_Pos_IsValid(a.from) && E2_Pos_IsValid(a.to) && E2_Pos_Equals(a.from, a.to) == false;
}

E2_Diff E2_Move_Diff(const E2_Move a) {
    return (E2_Diff){
        .row = (int)a.to.row - (int)a.from.row,
        .col = (int)a.to.col - (int)a.from.col,
    };
}

bool E2_Piece_IsEmpty(const E2_Piece p) { return p.type == E2_PIECE_TYPE_NONE; }

bool E2_Piece_Equals(const E2_Piece a, const E2_Piece b) { return a.type == b.type && a.side == b.side; }

bool E2_Board_MoveResult_Equals(const E2_Board_MoveResult* a, const E2_Board_MoveResult* b) {
    assert(a != nullptr);
    assert(b != nullptr);
    return a->err == b->err && E2_Piece_Equals(a->pieceTaken, b->pieceTaken) &&
           E2_Pos_Equals(a->obstacleAt, b->obstacleAt);
}

void E2_Board_PlacePieces(E2_Board* b) {
    assert(b != nullptr);

    b->pieces[0][0] = (E2_Piece){.type = E2_PIECE_TYPE_ROOK, .side = E2_SIDE_WHITE};
    b->pieces[0][7] = (E2_Piece){.type = E2_PIECE_TYPE_ROOK, .side = E2_SIDE_WHITE};
    b->pieces[0][1] = (E2_Piece){.type = E2_PIECE_TYPE_KNIGHT, .side = E2_SIDE_WHITE};
    b->pieces[0][6] = (E2_Piece){.type = E2_PIECE_TYPE_KNIGHT, .side = E2_SIDE_WHITE};
    b->pieces[0][2] = (E2_Piece){.type = E2_PIECE_TYPE_BISHOP, .side = E2_SIDE_WHITE};
    b->pieces[0][5] = (E2_Piece){.type = E2_PIECE_TYPE_BISHOP, .side = E2_SIDE_WHITE};
    b->pieces[0][3] = (E2_Piece){.type = E2_PIECE_TYPE_QUEEN, .side = E2_SIDE_WHITE};
    b->pieces[0][4] = (E2_Piece){.type = E2_PIECE_TYPE_KING, .side = E2_SIDE_WHITE};

    b->pieces[7][0] = (E2_Piece){.type = E2_PIECE_TYPE_ROOK, .side = E2_SIDE_BLACK};
    b->pieces[7][7] = (E2_Piece){.type = E2_PIECE_TYPE_ROOK, .side = E2_SIDE_BLACK};
    b->pieces[7][1] = (E2_Piece){.type = E2_PIECE_TYPE_KNIGHT, .side = E2_SIDE_BLACK};
    b->pieces[7][6] = (E2_Piece){.type = E2_PIECE_TYPE_KNIGHT, .side = E2_SIDE_BLACK};
    b->pieces[7][2] = (E2_Piece){.type = E2_PIECE_TYPE_BISHOP, .side = E2_SIDE_BLACK};
    b->pieces[7][5] = (E2_Piece){.type = E2_PIECE_TYPE_BISHOP, .side = E2_SIDE_BLACK};
    b->pieces[7][3] = (E2_Piece){.type = E2_PIECE_TYPE_QUEEN, .side = E2_SIDE_BLACK};
    b->pieces[7][4] = (E2_Piece){.type = E2_PIECE_TYPE_KING, .side = E2_SIDE_BLACK};

    for (size_t i = 0; i < E2_SIDE_LEN; i++) {
        b->pieces[1][i] = (E2_Piece){.type = E2_PIECE_TYPE_PAWN, .side = E2_SIDE_WHITE};
        b->pieces[6][i] = (E2_Piece){.type = E2_PIECE_TYPE_PAWN, .side = E2_SIDE_BLACK};
    }

    for (size_t i = 2; i < 6; i++) {
        for (size_t j = 0; j < E2_SIDE_LEN; j++) {
            b->pieces[i][j].type = E2_PIECE_TYPE_NONE;
            b->pieces[i][j].side = E2_SIDE_NONE;
        }
    }
}

bool E2_Board_Equals(const E2_Board* a, const E2_Board* b) {
    assert(a != nullptr);
    assert(b != nullptr);
    for (size_t i = 0; i < E2_SIDE_LEN; i++) {
        for (size_t j = 0; j < E2_SIDE_LEN; j++) {
            const auto pieceA = a->pieces[i][j];
            const auto pieceB = b->pieces[i][j];
            if (!E2_Piece_Equals(pieceA, pieceB)) {
                return false;
            }
        }
    }

    return true;
}

E2_Board_ParseResult E2_Board_Parse(E2_Board* b, const char src[1]) {
    assert(b != nullptr);
    assert(src != nullptr);

    if (strnlen(src, E2_TOTAL_CELLS + 1) < E2_TOTAL_CELLS) {
        return (E2_Board_ParseResult){.err = E2_BOARD_PARSE_ERROR_TOO_SHORT};
    }

    *b            = (E2_Board){};
    size_t offset = 0;
    for (size_t i = 0; i < E2_TOTAL_CELLS; i++) {
        const auto ch = src[i];
        offset++;

        E2_Piece piece = {};
        if (!E2_Piece_FromChar(&piece, ch)) {
            return (E2_Board_ParseResult
            ){.err = E2_BOARD_PARSE_ERROR_UNEXPECTED_CHAR, .offset = offset, .invalidChar = ch};
        }

        if (E2_Piece_IsEmpty(piece)) {
            continue;
        }

        const E2_Pos pos  = {.row = i / E2_SIDE_LEN, .col = i % E2_SIDE_LEN};
        E2_Piece*    cell = E2_Board_AtRef(b, pos);
        *cell             = piece;
    }

    return (E2_Board_ParseResult){.offset = offset};
}

E2_Piece E2_Board_At(const E2_Board* b, const E2_Pos pos) {
    assert(b != nullptr);
    assert(E2_Pos_IsValid(pos));
    return b->pieces[pos.row][pos.col];
}

E2_Piece* E2_Board_AtRef(E2_Board* b, const E2_Pos pos) {
    assert(b != nullptr);
    assert(E2_Pos_IsValid(pos));
    return &b->pieces[pos.row][pos.col];
}

E2_Board_MoveResult E2_Board_Move(E2_Board* b, const E2_Move m) {
    assert(b != nullptr);
    assert(E2_Move_IsValid(m));

    const auto piece = E2_Board_At(b, m.from);
    switch (piece.type) {
        case E2_PIECE_TYPE_PAWN:
            return E2_Board_MovePawn(b, m);
        case E2_PIECE_TYPE_ROOK:
            return E2_Board_MoveRook(b, m);
        case E2_PIECE_TYPE_KNIGHT:
            return E2_Board_MoveKnight(b, m);
        case E2_PIECE_TYPE_BISHOP:
            return E2_Board_MoveBishop(b, m);
        case E2_PIECE_TYPE_QUEEN:
            return E2_Board_MoveQueen(b, m);
        case E2_PIECE_TYPE_KING:
            return E2_Board_MoveKing(b, m);
        case E2_PIECE_TYPE_NONE:
        default:
            return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_NO_PIECE};
    }
}

int sign(const int a) { return a > 0 ? 1 : a < 0 ? -1 : 0; }

E2_Board_MoveResult E2_Board_MovePawn(E2_Board*, E2_Move) {
    return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_ILLEGAL};
}

E2_Board_MoveResult E2_Board_MoveRook(E2_Board* b, const E2_Move m) {
    const auto cellFrom = E2_Board_AtRef(b, m.from);
    const auto cellTo   = E2_Board_AtRef(b, m.to);
    const auto side     = cellFrom->side;
    const auto diff     = E2_Move_Diff(m);

    if (diff.row != 0 && diff.col != 0) {
        return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_ILLEGAL};
    }

    const auto distance   = MaxSizeT((size_t)abs(diff.row), (size_t)abs(diff.col));
    E2_Piece   pieceTaken = {};
    for (size_t i = 1; i < distance + 1; i++) {
        const E2_Pos p    = {.row = m.from.row + sign(diff.row) * i, .col = m.from.col + sign(diff.col) * i};
        const auto   cell = E2_Board_At(b, p);
        if (E2_Piece_IsEmpty(cell)) {
            continue;
        }

        if (i == distance && cell.side != side) {
            pieceTaken = cell;
            break;
        }

        return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_OBSTACLE, .obstacleAt = p};
    }

    *cellTo   = *cellFrom;
    *cellFrom = (E2_Piece){};

    return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_OK, .pieceTaken = pieceTaken};
}

E2_Board_MoveResult E2_Board_MoveKnight(E2_Board* b, const E2_Move m) {
    assert(b != nullptr);
    assert(E2_Move_IsValid(m));

    const auto cellFrom = E2_Board_AtRef(b, m.from);
    const auto cellTo   = E2_Board_AtRef(b, m.to);
    const auto side     = cellFrom->side;
    const auto diff     = E2_Move_Diff(m);

    if ((abs(diff.row) != 2 || abs(diff.col) != 1) && (abs(diff.row) != 1 || abs(diff.col) != 2)) {
        return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_ILLEGAL};
    }

    E2_Piece pieceTaken = {};
    if (!E2_Piece_IsEmpty(*cellTo)) {
        if (cellTo->side == side) {
            return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_OBSTACLE, .obstacleAt = m.to};
        }

        pieceTaken = *cellTo;
    }

    *cellTo   = *cellFrom;
    *cellFrom = (E2_Piece){};

    return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_OK, .pieceTaken = pieceTaken};
}

E2_Board_MoveResult E2_Board_MoveBishop(E2_Board* b, const E2_Move m) {
    assert(b != nullptr);
    assert(E2_Move_IsValid(m));

    const auto cellFrom = E2_Board_AtRef(b, m.from);
    const auto cellTo   = E2_Board_AtRef(b, m.to);
    const auto side     = cellFrom->side;
    const auto diff     = E2_Move_Diff(m);

    if (abs(diff.row) != abs(diff.col)) {
        return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_ILLEGAL};
    }

    const auto distance   = (size_t)abs(diff.row);
    E2_Piece   pieceTaken = {};
    for (size_t i = 1; i < distance + 1; i++) {
        const E2_Pos p    = {.row = m.from.row + sign(diff.row) * i, .col = m.from.col + sign(diff.col) * i};
        const auto   cell = E2_Board_At(b, p);
        if (E2_Piece_IsEmpty(cell)) {
            continue;
        }

        if (i == distance && cell.side != side) {
            pieceTaken = cell;
            break;
        }

        return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_OBSTACLE, .obstacleAt = p};
    }

    *cellTo   = *cellFrom;
    *cellFrom = (E2_Piece){};

    return (E2_Board_MoveResult){.err = E2_BOARD_MOVE_OK, .pieceTaken = pieceTaken};
}

E2_Board_MoveResult E2_Board_MoveQueen(E2_Board* b, const E2_Move m) {
    assert(b != nullptr);
    assert(E2_Move_IsValid(m));

    const auto moveRookResult = E2_Board_MoveRook(b, m);
    if (moveRookResult.err == E2_BOARD_MOVE_OK) {
        return moveRookResult;
    }

    const auto moveBishopResult = E2_Board_MoveBishop(b, m);
    if (moveBishopResult.err == E2_BOARD_MOVE_OK) {
        return moveBishopResult;
    }

    return (E2_Board_MoveResult){
        .err = E2_BOARD_MOVE_ILLEGAL,
    };
}

E2_Board_MoveResult E2_Board_MoveKing(E2_Board* b, const E2_Move m) {
    assert(b != nullptr);
    assert(E2_Move_IsValid(m));

    assert(false);
}
