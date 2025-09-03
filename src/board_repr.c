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

size_t CharSlice_WritePiece(CharSlice* dst, const Piece p) {
    assert(dst != nullptr);

    size_t written = 0;
    written += CharSlice_WriteChar(dst, '{');
    written += CharSlice_WritePieceType(dst, p.type);
    written += CharSlice_WriteChar(dst, ',');
    written += CharSlice_WriteSide(dst, p.side);
    written += CharSlice_WriteChar(dst, '}');
    return written;
}

PieceType PieceType_Parse(const CharSlice src) {
    assert(CharSlice_IsValid(src));
    if (src.len < 1) {
        return PIECE_TYPE_UNSPECIFIED;
    }

    if (CharSlice_Equals(src, CHAR_SLICE("PAWN"))) {
        return PIECE_TYPE_PAWN;
    }

    if (CharSlice_Equals(src, CHAR_SLICE("ROOK"))) {
        return PIECE_TYPE_ROOK;
    }

    if (CharSlice_Equals(src, CHAR_SLICE("KNIGHT"))) {
        return PIECE_TYPE_KNIGHT;
    }

    if (CharSlice_Equals(src, CHAR_SLICE("BISHOP"))) {
        return PIECE_TYPE_BISHOP;
    }

    if (CharSlice_Equals(src, CHAR_SLICE("QUEEN"))) {
        return PIECE_TYPE_QUEEN;
    }
    if (CharSlice_Equals(src, CHAR_SLICE("KING"))) {
        return PIECE_TYPE_KING;
    }

    return PIECE_TYPE_UNSPECIFIED;
}

Side Side_Parse(CharSlice src) {
    if (src.len < 1) {
        return SIDE_UNSPECIFIED;
    }

    const auto firstChar = CharSlice_At(src, 0);

    if (firstChar == 'B' && CharSlice_Cmp(src, CHAR_SLICE("BLACK")) == 0) {
        return SIDE_BLACK;
    }

    if (firstChar == 'W' && CharSlice_Cmp(src, CHAR_SLICE("WHITE")) == 0) {
        return SIDE_WHITE;
    }

    return SIDE_UNSPECIFIED;
}

size_t CharSlice_WritePieceType(CharSlice* dst, const PieceType t) {
    assert(dst != nullptr);
    switch (t) {
        case PIECE_TYPE_PAWN:
            return CharSlice_Write(dst, CHAR_SLICE("PAWN"));
        case PIECE_TYPE_ROOK:
            return CharSlice_Write(dst, CHAR_SLICE("ROOK"));
        case PIECE_TYPE_KNIGHT:
            return CharSlice_Write(dst, CHAR_SLICE("KNIGHT"));
        case PIECE_TYPE_BISHOP:
            return CharSlice_Write(dst, CHAR_SLICE("BISHOP"));
        case PIECE_TYPE_QUEEN:
            return CharSlice_Write(dst, CHAR_SLICE("QUEEN"));
        case PIECE_TYPE_KING:
            return CharSlice_Write(dst, CHAR_SLICE("KING"));
        case PIECE_TYPE_UNSPECIFIED:
            return CharSlice_Write(dst, CHAR_SLICE("NONE"));
        default:
            return 0;
    }
}

size_t CharSlice_WriteSide(CharSlice* dst, const Side s) {
    assert(dst != nullptr);
    switch (s) {
        case SIDE_WHITE:
            return CharSlice_Write(dst, CHAR_SLICE("WHITE"));
        case SIDE_BLACK:
            return CharSlice_Write(dst, CHAR_SLICE("BLACK"));
        default:
            return 0;
    }
}

PosParseResult Pos_Parse(Pos* dst, const CharSlice src) {
    assert(dst != nullptr);

    if (src.len < POS_STR_LEN) {
        return (PosParseResult){.err = POS_PARSE_ERR_TOO_SHORT};
    }

    const auto colChar = CharSlice_At(src, 0);
    const auto rowChar = CharSlice_At(src, 1);
    if (colChar < COL_CHAR_MIN || colChar > COL_CHAR_MAX || rowChar < ROW_CHAR_MIN || rowChar > ROW_CHAR_MAX) {
        return (PosParseResult){.err = POS_PARSE_ERR_INVALID_FORMAT};
    }

    dst->col = colChar - COL_CHAR_MIN;
    dst->row = BOARD_SIDE_LEN - (rowChar - ROW_CHAR_MIN) - 1;
    assert(Pos_IsValid(*dst));
    return (PosParseResult){.offset = 2};
}

size_t CharSlice_WritePosParseErr(CharSlice* dst, const PosParseErr err) {
    switch (err) {
        case POS_PARSE_ERR_OK:
            return CharSlice_Write(dst, CHAR_SLICE("OK"));
        case POS_PARSE_ERR_TOO_SHORT:
            return CharSlice_Write(dst, CHAR_SLICE("TOO_SHORT"));
        case POS_PARSE_ERR_INVALID_FORMAT:
            return CharSlice_Write(dst, CHAR_SLICE("INVALID_FORMAT"));
        default:
            return CharSlice_WriteF(dst, "UNKNOWN (%d)", err);
    }
}

bool PosParseResult_Equals(const PosParseResult a, const PosParseResult b) {
    return a.err == b.err && a.offset == b.offset;
}

size_t CharSlice_WritePosParseResult(CharSlice* dst, const PosParseResult a) {
    assert(dst != nullptr);
    size_t written = 0;
    written += CharSlice_WriteChar(dst, '{');
    written += CharSlice_WritePosParseErr(dst, a.err);
    written += CharSlice_WriteChar(dst, ',');
    written += CharSlice_WriteF(dst, "%zd", a.offset);
    written += CharSlice_WriteChar(dst, '}');
    return written;
}

size_t CharSlice_WritePos(CharSlice* dst, const Pos a) {
    assert(dst != nullptr);
    assert(Pos_IsValid(a));
    const char col = (char)(COL_CHAR_MIN + a.col);
    const char row = (char)(ROW_CHAR_MIN + BOARD_SIDE_LEN - a.row - 1);
    return CharSlice_WriteF(dst, "%c%c", col, row);
}

size_t CharSlice_WriteMoveParseErr(CharSlice* dst, const MoveParseErr err) {
    switch (err) {
        case MOVE_PARSE_ERR_OK:
            return CharSlice_Write(dst, CHAR_SLICE("OK"));
        case MOVE_PARSE_ERR_TOO_SHORT:
            return CharSlice_Write(dst, CHAR_SLICE("TOO_SHORT"));
        case MOVE_PARSE_ERR_INVALID_FROM_FORMAT:
            return CharSlice_Write(dst, CHAR_SLICE("INVALID_FROM_FORMAT"));
        case MOVE_PARSE_ERR_INVALID_TO_FORMAT:
            return CharSlice_Write(dst, CHAR_SLICE("INVALID_TO_FORMAT"));
        default:
            return CharSlice_WriteF(dst, "UNKNOWN (%d)", err);
    }
}

bool MoveParseResult_Equals(const MoveParseResult a, const MoveParseResult b) {
    return a.err == b.err && a.offset == b.offset;
}

size_t CharSlice_WriteMoveParseResult(CharSlice* dst, const MoveParseResult a) {
    assert(dst != nullptr);
    size_t written = 0;
    written += CharSlice_WriteChar(dst, '{');
    written += CharSlice_WriteMoveParseErr(dst, a.err);
    written += CharSlice_WriteChar(dst, ',');
    written += CharSlice_WriteF(dst, "%zd", a.offset);
    written += CharSlice_WriteChar(dst, '}');
    return written;
}

MoveParseResult Move_Parse(Move* dst, const CharSlice src) {
    assert(dst != nullptr);

    if (src.len < MOVE_STR_LEN) {
        return (MoveParseResult){.err = MOVE_PARSE_ERR_TOO_SHORT};
    }

    Pos        from, to;
    const auto fromParseResult = Pos_Parse(&from, src);
    if (fromParseResult.err != POS_PARSE_ERR_OK) {
        return (MoveParseResult){.err = MOVE_PARSE_ERR_INVALID_FROM_FORMAT};
    }

    const auto toPosSlice    = CharSlice_View(src, fromParseResult.offset, src.len);
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

size_t CharSlice_WriteMove(CharSlice* dst, const Move a) {
    assert(dst != nullptr);
    assert(Move_IsValid(a));

    size_t written = 0;
    written += CharSlice_WritePos(dst, a.from);
    written += CharSlice_WritePos(dst, a.to);
    return written;
}

size_t CharSlice_WriteBoardParseErr(CharSlice* dst, const SquaresParseErr err) {
    switch (err) {
        case SQUARES_PARSE_ERR_OK:
            return CharSlice_Write(dst, CHAR_SLICE("OK"));
        case SQUARES_PARSE_ERR_TOO_SHORT:
            return CharSlice_Write(dst, CHAR_SLICE("TOO_SHORT"));
        case SQUARES_PARSE_ERR_UNEXPECTED_CHAR:
            return CharSlice_Write(dst, CHAR_SLICE("UNEXPECTED_CHAR"));
        default:
            return CharSlice_WriteF(dst, "UNKNOWN (%d)", err);
    }
}

bool SquaresParseResult_Equals(const SquaresParseResult a, const SquaresParseResult b) {
    return a.err == b.err && a.offset == b.offset && a.unexpectedChar == b.unexpectedChar;
}

size_t CharSlice_WriteBoardParseResult(CharSlice* dst, const SquaresParseResult err) {
    assert(dst != nullptr);
    size_t written = 0;
    written += CharSlice_WriteChar(dst, '{');
    written += CharSlice_WriteBoardParseErr(dst, err.err);
    written += CharSlice_WriteChar(dst, ',');
    written += CharSlice_WriteF(dst, "%zd", err.offset);
    if (err.err == SQUARES_PARSE_ERR_UNEXPECTED_CHAR) {
        written += CharSlice_WriteChar(dst, ',');
        written += CharSlice_WriteChar(dst, err.unexpectedChar);
    }
    written += CharSlice_WriteChar(dst, '}');
    return written;
}

size_t CharSlice_WriteMoveError(CharSlice* dst, const MoveErr a) {
    switch (a) {
        case MOVE_ERR_OK:
            return CharSlice_Write(dst, CHAR_SLICE("OK"));
        case MOVE_ERR_NO_PIECE:
            return CharSlice_Write(dst, CHAR_SLICE("NO_PIECE"));
        case MOVE_ERR_NO_MOVEMENT:
            return CharSlice_Write(dst, CHAR_SLICE("NO_MOVEMENT"));
        case MOVE_ERR_ILLEGAL_MOVE:
            return CharSlice_Write(dst, CHAR_SLICE("ILLEGAL_MOVE"));
        case MOVE_ERR_OBSTACLE:
            return CharSlice_Write(dst, CHAR_SLICE("OBSTACLE"));
        case MOVE_ERR_UNDER_THREAT:
            return CharSlice_Write(dst, CHAR_SLICE("UNDER_THREAT"));
        default:
            return CharSlice_WriteF(dst, "UNKNOWN (%d)", a);
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

size_t CharSlice_WriteMoveResult(CharSlice* dst, const MoveResult* a) {
    assert(dst != nullptr);
    assert(a != nullptr);

    size_t written = 0;

    written += CharSlice_WriteChar(dst, '{');
    written += CharSlice_WriteMoveError(dst, a->err);

    if (a->err == MOVE_ERR_OBSTACLE) {
        written += CharSlice_WriteChar(dst, ',');
        written += CharSlice_WritePos(dst, a->obstacleAt);
    }

    if (!Piece_IsEmpty(a->pieceTaken)) {
        written += CharSlice_WriteChar(dst, ',');
        written += CharSlice_WritePiece(dst, a->pieceTaken);
    }

    written += CharSlice_WriteChar(dst, '}');
    return written;
}

SquaresParseResult Squares_Parse(Squares dst, const CharSlice src) {
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

        const auto ch = CharSlice_At(src, offset++);
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

size_t CharSlice_WriteBoard(CharSlice* dst, const Board* b) {
    size_t written = 0;
    for (size_t i = 0; i < BOARD_SIDE_LEN; ++i) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; ++j) {
            const Pos  pos   = {.row = i, .col = j};
            const auto piece = Squares_At(b->squares, pos);
            char       pieceChar;
            if (piece.side == SIDE_WHITE) {
                pieceChar = PieceType_ToUpperCaseChar(piece.type);
            } else {
                pieceChar = PieceType_ToLowerCaseChar(piece.type);
            }
            written += CharSlice_WriteChar(dst, pieceChar);
        }
        written += CharSlice_WriteChar(dst, '\n');
    }

    return written;
}
