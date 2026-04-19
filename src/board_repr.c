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
static const char UNICODE_WHITE_KING[4]   = "\u2654";
static const char UNICODE_WHITE_QUEEN[4]  = "\u2655";
static const char UNICODE_WHITE_ROOK[4]   = "\u2656";
static const char UNICODE_WHITE_BISHOP[4] = "\u2657";
static const char UNICODE_WHITE_KNIGHT[4] = "\u2658";
static const char UNICODE_WHITE_PAWN[4]   = "\u2659";

// Black pieces
static const char UNICODE_BLACK_KING[4]   = "\u265A";
static const char UNICODE_BLACK_QUEEN[4]  = "\u265B";
static const char UNICODE_BLACK_ROOK[4]   = "\u265C";
static const char UNICODE_BLACK_BISHOP[4] = "\u265D";
static const char UNICODE_BLACK_KNIGHT[4] = "\u265E";
static const char UNICODE_BLACK_PAWN[4]   = "\u265F";

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
                return " ";
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
            return " ";
    }
}

size_t CharBuff_WritePiece(CharBuff* dst, const Piece p) {
    assert(dst != NULL);

    size_t written = 0;
    written += CharBuff_WriteChar(dst, '{');
    written += CharBuff_WritePieceType(dst, p.type);
    written += CharBuff_WriteChar(dst, ',');
    written += CharBuff_WriteSide(dst, p.side);
    written += CharBuff_WriteChar(dst, '}');
    return written;
}

PieceTypeParseResult PieceType_Parse(PieceType* dst, const Str src) {
    assert(dst != NULL);
    assert(Str_IsValid(src));
    assert(src.len > 0);

    typedef struct {
        PieceType type;
        Str       str;
    } map;

    const map mapping[] = {
        {PIECE_TYPE_PAWN, STR("PAWN")},     {PIECE_TYPE_ROOK, STR("ROOK")},   {PIECE_TYPE_KNIGHT, STR("KNIGHT")},
        {PIECE_TYPE_BISHOP, STR("BISHOP")}, {PIECE_TYPE_QUEEN, STR("QUEEN")}, {PIECE_TYPE_KING, STR("KING")},
    };

    for (size_t i = 0; i < sizeof(mapping) / sizeof(map); i++) {
        if (Str_Equals(src, mapping[i].str)) {
            *dst = mapping[i].type;
            return (PieceTypeParseResult){.offset = mapping[i].str.len};
        }
    }

    return (PieceTypeParseResult){.err = PIECE_TYPE_PARSE_ERR_INVALID_VALUE};
}

SideParseResult Side_Parse(Side* dst, const Str src) {
    assert(dst != NULL);
    assert(src.len > 0);

    const Str BLACK_STR = STR("BLACK");
    const Str WHITE_STR = STR("WHITE");

    if (Str_Equals(src, BLACK_STR)) {
        *dst = SIDE_BLACK;
        return (SideParseResult){.offset = WHITE_STR.len};
        ;
    }
    if (Str_Equals(src, WHITE_STR)) {
        *dst = SIDE_WHITE;
        return (SideParseResult){.offset = WHITE_STR.len};
        ;
    }

    return (SideParseResult){.err = SIDE_PARSE_ERR_UNEXPECTED_VALUE};
}

size_t CharBuff_WritePieceType(CharBuff* dst, const PieceType t) {
    assert(dst != NULL);
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
            return CharBuff_WriteStr(dst, STR("UNSPECIFIED"));
        default:
            return CharBuff_WriteF(dst, "INVALID (%d)", t);
    }
}

size_t CharBuff_WriteSide(CharBuff* dst, const Side s) {
    assert(dst != NULL);
    switch (s) {
        case SIDE_WHITE:
            return CharBuff_WriteStr(dst, STR("WHITE"));
        case SIDE_BLACK:
            return CharBuff_WriteStr(dst, STR("BLACK"));
        default:
            return CharBuff_WriteF(dst, "INVALID (%d)", s);
            ;
    }
}

PosParseResult Pos_Parse(Pos* dst, const Str src) {
    assert(dst != NULL);

    if (src.len < POS_STR_LEN) {
        return (PosParseResult){.err = POS_PARSE_ERR_TOO_SHORT};
    }

    const char colChar = Str_At(src, 0);
    const char rowChar = Str_At(src, 1);
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
            return CharBuff_WriteF(dst, "INVALID (%d)", err);
    }
}

bool PosParseResult_Equals(const PosParseResult a, const PosParseResult b) {
    return a.err == b.err && a.offset == b.offset;
}

size_t CharBuff_WritePosParseResult(CharBuff* dst, const PosParseResult a) {
    assert(dst != NULL);
    size_t written = 0;
    written += CharBuff_WriteChar(dst, '{');
    written += CharBuff_WritePosParseErr(dst, a.err);
    written += CharBuff_WriteChar(dst, ',');
    written += CharBuff_WriteF(dst, "%zd", a.offset);
    written += CharBuff_WriteChar(dst, '}');
    return written;
}

size_t CharBuff_WritePos(CharBuff* dst, const Pos a) {
    assert(dst != NULL);
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
            return CharBuff_WriteF(dst, "INVALID (%d)", err);
    }
}

bool MoveParseResult_Equals(const MoveParseResult a, const MoveParseResult b) {
    return a.err == b.err && a.offset == b.offset;
}

size_t CharBuff_WriteMoveParseResult(CharBuff* dst, const MoveParseResult a) {
    assert(dst != NULL);
    size_t written = 0;
    written += CharBuff_WriteChar(dst, '{');
    written += CharBuff_WriteMoveParseErr(dst, a.err);
    written += CharBuff_WriteChar(dst, ',');
    written += CharBuff_WriteF(dst, "%zd", a.offset);
    written += CharBuff_WriteChar(dst, '}');
    return written;
}

MoveParseResult Move_Parse(Move* dst, const Str src) {
    assert(dst != NULL);

    if (src.len < MOVE_STR_LEN) {
        return (MoveParseResult){.err = MOVE_PARSE_ERR_TOO_SHORT};
    }

    Pos                  from, to;
    const PosParseResult fromParseResult = Pos_Parse(&from, src);
    if (fromParseResult.err != POS_PARSE_ERR_OK) {
        return (MoveParseResult){.err = MOVE_PARSE_ERR_INVALID_FROM_FORMAT};
    }

    const Str            toPosStr      = Str_View(src, fromParseResult.offset, src.len);
    const PosParseResult toParseResult = Pos_Parse(&to, toPosStr);
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
    assert(dst != NULL);
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
            return CharBuff_WriteF(dst, "INVALID (%d)", err);
    }
}

bool SquaresParseResult_Equals(const SquaresParseResult a, const SquaresParseResult b) {
    return a.err == b.err && a.offset == b.offset && a.unexpectedChar == b.unexpectedChar;
}

size_t CharBuff_WriteBoardParseResult(CharBuff* dst, const SquaresParseResult err) {
    assert(dst != NULL);
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
            return CharBuff_WriteF(dst, "INVALID (%d)", a);
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
    assert(dst != NULL);
    assert(a != NULL);

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
    assert(dst != NULL);

    size_t i      = 0;
    size_t offset = 0;
    while (i < BOARD_SIZE) {
        if (offset >= src.len) {
            return (SquaresParseResult){
                .err    = SQUARES_PARSE_ERR_TOO_SHORT,
                .offset = offset,
            };
        }

        const char ch = Str_At(src, offset++);
        if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t') {
            continue;
        }

        Piece piece = {0};
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
            const Pos   pos   = {.row = i, .col = j};
            const Piece piece = Squares_At(ss, pos);
            char        pieceChar;
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
