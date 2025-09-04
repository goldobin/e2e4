#include "board_repr.h"

#include <assert.h>

char PieceType_ToUpperCaseChar(const PieceType t) {
    switch (t) {
        case PIECE_TYPE_PAWN:
            return 'P';
        case PIECE_TYPE_ROOK:
            return 'R';
        case PIECE_TYPE_KNIGHT:
            return 'N';
        case PIECE_TYPE_BISHOP:
            return 'B';
        case PIECE_TYPE_QUEEN:
            return 'Q';
        case PIECE_TYPE_KING:
            return 'K';
        default:
            return '.';
    }
}

char PieceType_ToLowerCaseChar(const PieceType t) {
    switch (t) {
        case PIECE_TYPE_PAWN:
            return 'p';
        case PIECE_TYPE_ROOK:
            return 'r';
        case PIECE_TYPE_KNIGHT:
            return 'n';
        case PIECE_TYPE_BISHOP:
            return 'b';
        case PIECE_TYPE_QUEEN:
            return 'q';
        case PIECE_TYPE_KING:
            return 'k';
        default:
            return '.';
    }
}

// White pieces
constexpr char UNICODE_WHITE_KING[4]   = "\u2654";
constexpr char UNICODE_WHITE_QUEEN[4]  = "\u2655";
constexpr char UNICODE_WHITE_ROOK[4]   = "\u2656";
constexpr char UNICODE_WHITE_BISHOP[4] = "\u2657";
constexpr char UNICODE_WHITE_KNIGHT[4] = "\u2658";
constexpr char UNICODE_WHITE_PAWN[4]   = "\u2659";

// Black pieces
constexpr char UNICODE_BLACK_KING[4]   = "\u265A";
constexpr char UNICODE_BLACK_QUEEN[4]  = "\u265B";
constexpr char UNICODE_BLACK_ROOK[4]   = "\u265C";
constexpr char UNICODE_BLACK_BISHOP[4] = "\u265D";
constexpr char UNICODE_BLACK_KNIGHT[4] = "\u265E";
constexpr char UNICODE_BLACK_PAWN[4]   = "\u265F";

const char* Piece_ToUnicodeChar(Piece p) {
    if (p.side == SIDE_WHITE) {
        switch (p.type) {
            case PIECE_TYPE_KING:
                return UNICODE_WHITE_KING;
            case PIECE_TYPE_QUEEN:
                return UNICODE_WHITE_QUEEN;
            case PIECE_TYPE_ROOK:
                return UNICODE_WHITE_ROOK;
            case PIECE_TYPE_BISHOP:
                return UNICODE_WHITE_BISHOP;
            case PIECE_TYPE_KNIGHT:
                return UNICODE_WHITE_KNIGHT;
            case PIECE_TYPE_PAWN:
                return UNICODE_WHITE_PAWN;
            default:
                return ".";
        }
    }
    switch (p.type) {
        case PIECE_TYPE_KING:
            return UNICODE_BLACK_KING;
        case PIECE_TYPE_QUEEN:
            return UNICODE_BLACK_QUEEN;
        case PIECE_TYPE_ROOK:
            return UNICODE_BLACK_ROOK;
        case PIECE_TYPE_BISHOP:
            return UNICODE_BLACK_BISHOP;
        case PIECE_TYPE_KNIGHT:
            return UNICODE_BLACK_KNIGHT;
        case PIECE_TYPE_PAWN:
            return UNICODE_BLACK_PAWN;
        default:
            return ".";
    }
}

size_t CharBuff_WritePiece(CharBuff* dst, const Piece p) {
    assert(dst != nullptr);

    size_t written = 0;
    written += CharBuff_WriteChar(dst, '{');
    written += CharBuff_WritePieceType(dst, p.type);
    written += CharBuff_WriteChar(dst, ',');
    written += CharBuff_WriteSide(dst, p.side);
    written += CharBuff_WriteChar(dst, '}');
    return written;
}

PieceType PieceType_Parse(const Str src) {
    assert(Str_IsValid(src));
    if (src.len < 1) {
        return PIECE_TYPE_UNSPECIFIED;
    }

    if (Str_Equals(src, STR("PAWN"))) {
        return PIECE_TYPE_PAWN;
    }

    if (Str_Equals(src, STR("ROOK"))) {
        return PIECE_TYPE_ROOK;
    }

    if (Str_Equals(src, STR("KNIGHT"))) {
        return PIECE_TYPE_KNIGHT;
    }

    if (Str_Equals(src, STR("BISHOP"))) {
        return PIECE_TYPE_BISHOP;
    }

    if (Str_Equals(src, STR("QUEEN"))) {
        return PIECE_TYPE_QUEEN;
    }
    if (Str_Equals(src, STR("KING"))) {
        return PIECE_TYPE_KING;
    }

    return PIECE_TYPE_UNSPECIFIED;
}

Side Side_Parse(Str src) {
    if (src.len < 1) {
        return SIDE_UNSPECIFIED;
    }

    const auto firstChar = Str_At(src, 0);

    if (firstChar == 'B' && Str_Equals(src, STR("BLACK"))) {
        return SIDE_BLACK;
    }

    if (firstChar == 'W' && Str_Equals(src, STR("WHITE"))) {
        return SIDE_WHITE;
    }

    return SIDE_UNSPECIFIED;
}

size_t CharBuff_WritePieceType(CharBuff* dst, const PieceType t) {
    assert(dst != nullptr);
    switch (t) {
        case PIECE_TYPE_PAWN:
            return CharBuff_WriteStr(dst, STR("PAWN"));
        case PIECE_TYPE_ROOK:
            return CharBuff_WriteStr(dst, STR("ROOK"));
        case PIECE_TYPE_KNIGHT:
            return CharBuff_WriteStr(dst, STR("KNIGHT"));
        case PIECE_TYPE_BISHOP:
            return CharBuff_WriteStr(dst, STR("BISHOP"));
        case PIECE_TYPE_QUEEN:
            return CharBuff_WriteStr(dst, STR("QUEEN"));
        case PIECE_TYPE_KING:
            return CharBuff_WriteStr(dst, STR("KING"));
        case PIECE_TYPE_UNSPECIFIED:
            return CharBuff_WriteStr(dst, STR("NONE"));
        default:
            return 0;
    }
}

size_t CharBuff_WriteSide(CharBuff* dst, const Side s) {
    assert(dst != nullptr);
    switch (s) {
        case SIDE_WHITE:
            return CharBuff_WriteStr(dst, STR("WHITE"));
        case SIDE_BLACK:
            return CharBuff_WriteStr(dst, STR("BLACK"));
        default:
            return 0;
    }
}

Str Side_ToStr(Side s) {
    switch (s) {
        case SIDE_WHITE:
            return STR("WHITE");
        case SIDE_BLACK:
            return STR("BLACK");
        default:
            return STR("");
    }
}

PosParseResult Pos_Parse(Pos* dst, const Str src) {
    assert(dst != nullptr);

    if (src.len < POS_STR_LEN) {
        return (PosParseResult){.err = POS_PARSE_ERR_TOO_SHORT};
    }

    const auto colChar = Str_At(src, 0);
    const auto rowChar = Str_At(src, 1);
    if (colChar < COL_CHAR_MIN || colChar > COL_CHAR_MAX || rowChar < ROW_CHAR_MIN || rowChar > ROW_CHAR_MAX) {
        return (PosParseResult){.err = POS_PARSE_ERR_INVALID_FORMAT};
    }

    dst->col = colChar - COL_CHAR_MIN;
    dst->row = BOARD_SIDE_LEN - (rowChar - ROW_CHAR_MIN) - 1;
    assert(Pos_IsValid(*dst));
    return (PosParseResult){.offset = 2};
}

size_t CharBuff_WritePosParseErr(CharBuff* dst, const PosParseErr err) {
    switch (err) {
        case POS_PARSE_ERR_OK:
            return CharBuff_WriteStr(dst, STR("OK"));
        case POS_PARSE_ERR_TOO_SHORT:
            return CharBuff_WriteStr(dst, STR("TOO_SHORT"));
        case POS_PARSE_ERR_INVALID_FORMAT:
            return CharBuff_WriteStr(dst, STR("INVALID_FORMAT"));
        default:
            return CharBuff_WriteF(dst, "UNKNOWN (%d)", err);
    }
}

bool PosParseResult_Equals(const PosParseResult a, const PosParseResult b) {
    return a.err == b.err && a.offset == b.offset;
}

size_t CharBuff_WritePosParseResult(CharBuff* dst, const PosParseResult a) {
    assert(dst != nullptr);
    size_t written = 0;
    written += CharBuff_WriteChar(dst, '{');
    written += CharBuff_WritePosParseErr(dst, a.err);
    written += CharBuff_WriteChar(dst, ',');
    written += CharBuff_WriteF(dst, "%zd", a.offset);
    written += CharBuff_WriteChar(dst, '}');
    return written;
}

size_t CharBuff_WritePos(CharBuff* dst, const Pos a) {
    assert(dst != nullptr);
    assert(Pos_IsValid(a));
    const char col = (char)(COL_CHAR_MIN + a.col);
    const char row = (char)(ROW_CHAR_MIN + BOARD_SIDE_LEN - a.row - 1);
    return CharBuff_WriteF(dst, "%c%c", col, row);
}

size_t CharBuff_WriteMoveParseErr(CharBuff* dst, const MoveParseErr err) {
    switch (err) {
        case MOVE_PARSE_ERR_OK:
            return CharBuff_WriteStr(dst, STR("OK"));
        case MOVE_PARSE_ERR_TOO_SHORT:
            return CharBuff_WriteStr(dst, STR("TOO_SHORT"));
        case MOVE_PARSE_ERR_INVALID_FROM_FORMAT:
            return CharBuff_WriteStr(dst, STR("INVALID_FROM_FORMAT"));
        case MOVE_PARSE_ERR_INVALID_TO_FORMAT:
            return CharBuff_WriteStr(dst, STR("INVALID_TO_FORMAT"));
        default:
            return CharBuff_WriteF(dst, "UNKNOWN (%d)", err);
    }
}

bool MoveParseResult_Equals(const MoveParseResult a, const MoveParseResult b) {
    return a.err == b.err && a.offset == b.offset;
}

size_t CharBuff_WriteMoveParseResult(CharBuff* dst, const MoveParseResult a) {
    assert(dst != nullptr);
    size_t written = 0;
    written += CharBuff_WriteChar(dst, '{');
    written += CharBuff_WriteMoveParseErr(dst, a.err);
    written += CharBuff_WriteChar(dst, ',');
    written += CharBuff_WriteF(dst, "%zd", a.offset);
    written += CharBuff_WriteChar(dst, '}');
    return written;
}

MoveParseResult Move_Parse(Move* dst, const Str src) {
    assert(dst != nullptr);

    if (src.len < MOVE_STR_LEN) {
        return (MoveParseResult){.err = MOVE_PARSE_ERR_TOO_SHORT};
    }

    Pos        from, to;
    const auto fromParseResult = Pos_Parse(&from, src);
    if (fromParseResult.err != POS_PARSE_ERR_OK) {
        return (MoveParseResult){.err = MOVE_PARSE_ERR_INVALID_FROM_FORMAT};
    }

    const auto toPosSlice    = Str_View(src, fromParseResult.offset, src.len);
    const auto toParseResult = Pos_Parse(&to, toPosSlice);
    if (toParseResult.err != POS_PARSE_ERR_OK) {
        return (MoveParseResult){.err = MOVE_PARSE_ERR_INVALID_TO_FORMAT};
    }

    dst->from = from;
    dst->to   = to;
    return (MoveParseResult){
        .offset = fromParseResult.offset + toParseResult.offset,
    };
}

size_t CharBuff_WriteMove(CharBuff* dst, const Move a) {
    assert(dst != nullptr);
    assert(Move_IsValid(a));

    size_t written = 0;
    written += CharBuff_WritePos(dst, a.from);
    written += CharBuff_WritePos(dst, a.to);
    return written;
}

size_t CharBuff_WriteBoardParseErr(CharBuff* dst, const SquaresParseErr err) {
    switch (err) {
        case SQUARES_PARSE_ERR_OK:
            return CharBuff_WriteStr(dst, STR("OK"));
        case SQUARES_PARSE_ERR_TOO_SHORT:
            return CharBuff_WriteStr(dst, STR("TOO_SHORT"));
        case SQUARES_PARSE_ERR_UNEXPECTED_CHAR:
            return CharBuff_WriteStr(dst, STR("UNEXPECTED_CHAR"));
        default:
            return CharBuff_WriteF(dst, "UNKNOWN (%d)", err);
    }
}

bool SquaresParseResult_Equals(const SquaresParseResult a, const SquaresParseResult b) {
    return a.err == b.err && a.offset == b.offset && a.unexpectedChar == b.unexpectedChar;
}

size_t CharBuff_WriteBoardParseResult(CharBuff* dst, const SquaresParseResult err) {
    assert(dst != nullptr);
    size_t written = 0;
    written += CharBuff_WriteChar(dst, '{');
    written += CharBuff_WriteBoardParseErr(dst, err.err);
    written += CharBuff_WriteChar(dst, ',');
    written += CharBuff_WriteF(dst, "%zd", err.offset);
    if (err.err == SQUARES_PARSE_ERR_UNEXPECTED_CHAR) {
        written += CharBuff_WriteChar(dst, ',');
        written += CharBuff_WriteChar(dst, err.unexpectedChar);
    }
    written += CharBuff_WriteChar(dst, '}');
    return written;
}

size_t CharBuff_WriteMoveError(CharBuff* dst, const MoveErr a) {
    switch (a) {
        case MOVE_ERR_OK:
            return CharBuff_WriteStr(dst, STR("OK"));
        case MOVE_ERR_NO_PIECE:
            return CharBuff_WriteStr(dst, STR("NO_PIECE"));
        case MOVE_ERR_NO_MOVEMENT:
            return CharBuff_WriteStr(dst, STR("NO_MOVEMENT"));
        case MOVE_ERR_ILLEGAL_MOVE:
            return CharBuff_WriteStr(dst, STR("ILLEGAL_MOVE"));
        case MOVE_ERR_OBSTACLE:
            return CharBuff_WriteStr(dst, STR("OBSTACLE"));
        default:
            return CharBuff_WriteF(dst, "UNKNOWN (%d)", a);
    }
}

bool Piece_FromChar(Piece* t, const char ch) {
    switch (ch) {
        case 'P':
            t->type = PIECE_TYPE_PAWN;
            t->side = SIDE_WHITE;
            return true;
        case 'R':
            t->type = PIECE_TYPE_ROOK;
            t->side = SIDE_WHITE;
            return true;
        case 'N':
            t->type = PIECE_TYPE_KNIGHT;
            t->side = SIDE_WHITE;
            return true;
        case 'B':
            t->type = PIECE_TYPE_BISHOP;
            t->side = SIDE_WHITE;
            return true;
        case 'Q':
            t->type = PIECE_TYPE_QUEEN;
            t->side = SIDE_WHITE;
            return true;
        case 'K':
            t->type = PIECE_TYPE_KING;
            t->side = SIDE_WHITE;
            return true;
        case 'p':
            t->type = PIECE_TYPE_PAWN;
            t->side = SIDE_BLACK;
            return true;
        case 'r':
            t->type = PIECE_TYPE_ROOK;
            t->side = SIDE_BLACK;
            return true;
        case 'n':
            t->type = PIECE_TYPE_KNIGHT;
            t->side = SIDE_BLACK;
            return true;
        case 'b':
            t->type = PIECE_TYPE_BISHOP;
            t->side = SIDE_BLACK;
            return true;
        case 'q':
            t->type = PIECE_TYPE_QUEEN;
            t->side = SIDE_BLACK;
            return true;
        case 'k':
            t->type = PIECE_TYPE_KING;
            t->side = SIDE_BLACK;
            return true;
        case '.':
            t->type = PIECE_TYPE_UNSPECIFIED;
            t->side = SIDE_UNSPECIFIED;
            return true;
        default:
            return false;
    }
}

size_t CharBuff_WriteMoveResult(CharBuff* dst, const MoveResult* a) {
    assert(dst != nullptr);
    assert(a != nullptr);

    size_t written = 0;

    written += CharBuff_WriteChar(dst, '{');
    written += CharBuff_WriteMoveError(dst, a->err);

    if (a->err == MOVE_ERR_OBSTACLE) {
        written += CharBuff_WriteChar(dst, ',');
        written += CharBuff_WritePos(dst, a->obstacleAt);
    }

    if (a->taken != PIECE_TYPE_UNSPECIFIED) {
        written += CharBuff_WriteChar(dst, ',');
        written += CharBuff_WritePieceType(dst, a->taken);
    }

    written += CharBuff_WriteChar(dst, '}');
    return written;
}

SquaresParseResult Squares_Parse(Squares dst, const Str src) {
    assert(dst != nullptr);

    size_t i      = 0;
    size_t offset = 0;
    while (i < BOARD_SIZE) {
        if (offset >= src.len) {
            return (SquaresParseResult){
                .err    = SQUARES_PARSE_ERR_TOO_SHORT,
                .offset = offset,
            };
        }

        const auto ch = Str_At(src, offset++);
        if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t') {
            continue;
        }

        Piece piece = {};
        if (!Piece_FromChar(&piece, ch)) {
            return (SquaresParseResult){
                .err            = SQUARES_PARSE_ERR_UNEXPECTED_CHAR,
                .offset         = offset,
                .unexpectedChar = ch,
            };
        }

        const Pos pos = {.col = i % BOARD_SIDE_LEN, .row = i / BOARD_SIDE_LEN};
        Squares_UpdateAt(dst, pos, piece);
        i++;
    }

    return (SquaresParseResult){.offset = offset};
}

size_t CharBuff_WriteSquares(CharBuff* dst, const Squares ss) {
    size_t written = 0;
    for (size_t i = 0; i < BOARD_SIDE_LEN; ++i) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; ++j) {
            const Pos  pos   = {.row = i, .col = j};
            const auto piece = Squares_At(ss, pos);
            char       pieceChar;
            if (piece.side == SIDE_WHITE) {
                pieceChar = PieceType_ToUpperCaseChar(piece.type);
            } else {
                pieceChar = PieceType_ToLowerCaseChar(piece.type);
            }
            written += CharBuff_WriteChar(dst, pieceChar);
        }
        written += CharBuff_WriteChar(dst, '\n');
    }

    return written;
}
