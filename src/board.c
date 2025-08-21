#include "board.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "func.h"
#include "json.h"

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
            t->type = PIECE_TYPE_NONE;
            t->side = SIDE_NONE;
            return true;
        default:
            return false;
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
size_t CharSlice_WritePieceAsJson(CharSlice* dst, JsonStack* js, const Piece* p) {
    assert(dst != nullptr);
    assert(js != nullptr);

    auto pieceTypeSlice = CharSlice_Make(0, 32);
    auto sideSlice      = CharSlice_Make(0, 32);

    CharSlice_WritePieceType(&pieceTypeSlice, p->type);
    CharSlice_WriteSide(&pieceTypeSlice, p->side);

    size_t written = 0;
    written += CharSlice_WriteJsonStart(dst, js, '{');
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("type"));
    written += CharSlice_WriteJsonString(dst, js, pieceTypeSlice);
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("side"));
    written += CharSlice_WriteJsonString(dst, js, sideSlice);
    written += CharSlice_WriteJsonEnd(dst, js);
    return written;
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

const char* Piece_ToUnicodeChar(const Piece* p) {
    if (p->side == SIDE_WHITE) {
        switch (p->type) {
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
    switch (p->type) {
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
bool Piece_InterpretJson(Piece* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    const auto t = JsonTokens_At(src->tokens, src->index);
    if (t->type != JSON_TOKEN_TYPE_OBJECT) {
        return false;
    }

    if (t->childrenCount < 2) {
        return false;
    }

    for (size_t i = 0; i < t->childrenCount; ++i) {
        const auto keyToken = JsonTokens_At(src->tokens, ++(src->index));
        if (keyToken->childrenCount < 1) {
            return false;
        }

        const auto key = JsonToken_View(keyToken, src->charSlice);
        if (CharSlice_Equals(key, CHAR_SLICE("side"))) {
            const auto valueToken = JsonTokens_At(src->tokens, ++(src->index));
            if (valueToken->childrenCount != 0) {
                return false;
            }

            const auto value = JsonToken_View(valueToken, src->charSlice);
            dst->side        = Side_Parse(value);
        } else if (CharSlice_Equals(key, CHAR_SLICE("type"))) {
            const auto valueToken = JsonTokens_At(src->tokens, ++(src->index));
            if (valueToken->childrenCount != 0) {
                return false;
            }

            const auto value = JsonToken_View(valueToken, src->charSlice);
            dst->type        = PieceType_Parse(value);
        } else {
            src->index = JsonTokens_Skip(src->tokens, ++(src->index));
        }
    }

    return true;
}

bool PieceType_IsValid(const PieceType t) {
    switch (t) {
        case PIECE_TYPE_NONE:
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
PieceType PieceType_Parse(const CharSlice src) {
    assert(CharSlice_IsValid(&src));
    if (src.len < 1) {
        return PIECE_TYPE_NONE;
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

    return PIECE_TYPE_NONE;
}

Side Side_Parse(CharSlice src) {
    if (src.len < 1) {
        return SIDE_NONE;
    }

    const auto firstChar = CharSlice_At(&src, 0);

    if (firstChar == 'B' && CharSlice_Cmp(src, CHAR_SLICE("BLACK")) == 0) {
        return SIDE_BLACK;
    }

    if (firstChar == 'W' && CharSlice_Cmp(src, CHAR_SLICE("WHITE")) == 0) {
        return SIDE_WHITE;
    }

    return SIDE_NONE;
}

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
        case PIECE_TYPE_NONE:
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

bool Pos_IsValid(const Pos a) { return a.row < BOARD_SIDE_LEN && a.col < BOARD_SIDE_LEN; }

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

bool Pos_Equals(const Pos a, const Pos b) { return a.row == b.row && a.col == b.col; }

PosParseResult Pos_Parse(Pos* dst, const CharSlice src) {
    assert(dst != nullptr);

    if (src.len < POS_STR_LEN) {
        return (PosParseResult){.err = POS_PARSE_ERR_TOO_SHORT};
    }

    const auto colChar = CharSlice_At(&src, 0);
    const auto rowChar = CharSlice_At(&src, 1);
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

    const auto toPosSlice    = CharSlice_View(&src, fromParseResult.offset, src.len);
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

bool Move_Equals(const Move a, const Move b) { return Pos_Equals(a.from, b.from) && Pos_Equals(a.to, b.to); }

bool Move_IsValid(const Move a) {
    return Pos_IsValid(a.from) && Pos_IsValid(a.to) && Pos_Equals(a.from, a.to) == false;
}

size_t CharSlice_WriteMove(CharSlice* dst, const Move a) {
    assert(dst != nullptr);
    assert(Move_IsValid(a));

    size_t written = 0;
    written += CharSlice_WritePos(dst, a.from);
    written += CharSlice_WritePos(dst, a.to);
    return written;
}

size_t CharSlice_WriteBoardParseErr(CharSlice* dst, const BoardParseErr err) {
    switch (err) {
        case BOARD_PARSE_ERR_OK:
            return CharSlice_Write(dst, CHAR_SLICE("OK"));
        case BOARD_PARSE_ERR_TOO_SHORT:
            return CharSlice_Write(dst, CHAR_SLICE("TOO_SHORT"));
        case BOARD_PARSE_ERR_UNEXPECTED_CHAR:
            return CharSlice_Write(dst, CHAR_SLICE("UNEXPECTED_CHAR"));
        default:
            return CharSlice_WriteF(dst, "UNKNOWN (%d)", err);
    }
}

bool BoardParseResult_Equals(const BoardParseResult a, const BoardParseResult b) {
    return a.err == b.err && a.offset == b.offset && a.unexpectedChar == b.unexpectedChar;
}

size_t CharSlice_WriteBoardParseResult(CharSlice* dst, const BoardParseResult err) {
    assert(dst != nullptr);
    size_t written = 0;
    written += CharSlice_WriteChar(dst, '{');
    written += CharSlice_WriteBoardParseErr(dst, err.err);
    written += CharSlice_WriteChar(dst, ',');
    written += CharSlice_WriteF(dst, "%zd", err.offset);
    if (err.err == BOARD_PARSE_ERR_UNEXPECTED_CHAR) {
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
        case MOVE_ERR_ILLEGAL:
            return CharSlice_Write(dst, CHAR_SLICE("ILLEGAL"));
        case MOVE_ERR_OBSTACLE:
            return CharSlice_Write(dst, CHAR_SLICE("OBSTACLE"));
        case MOVE_ERR_UNDER_THREAT:
            return CharSlice_Write(dst, CHAR_SLICE("UNDER_THREAT"));
        default:
            return CharSlice_WriteF(dst, "UNKNOWN (%d)", a);
    }
}

bool Piece_IsEmpty(const Piece* p) { return p->type == PIECE_TYPE_NONE; }

bool Piece_Equals(const Piece* a, const Piece* b) { return a->type == b->type && a->side == b->side; }

bool MoveResult_Equals(const MoveResult* a, const MoveResult* b) {
    assert(a != nullptr);
    assert(b != nullptr);
    return a->err == b->err && Piece_Equals(&a->pieceTaken, &b->pieceTaken) && Pos_Equals(a->obstacleAt, b->obstacleAt);
}

bool PieceTypes_InterpretJson(PieceTypes* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    const auto t = JsonTokens_At(src->tokens, src->index);
    if (t->type != JSON_TOKEN_TYPE_ARRAY) {
        // TODO: Add proper error handling
        return false;
    }

    if (!PieceTypes_Resize(dst, t->childrenCount)) {
        return false;
    }

    for (size_t i = 0; i < t->childrenCount; i++) {
        const auto entryToken = JsonTokens_At(src->tokens, ++src->index);
        if (entryToken->type != JSON_TOKEN_TYPE_STRING || entryToken->childrenCount != 0) {
            return false;
        }
        const auto entry       = JsonToken_View(entryToken, src->charSlice);
        const auto pieceType   = PieceType_Parse(entry);
        *PieceTypes_At(dst, i) = pieceType;
    }

    return true;
}
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

    if (!Piece_IsEmpty(&a->pieceTaken)) {
        written += CharSlice_WriteChar(dst, ',');
        written += CharSlice_WritePiece(dst, a->pieceTaken);
    }

    written += CharSlice_WriteChar(dst, '}');
    return written;
}
size_t CharSlice_WriteSquaresAsJson(CharSlice* dst, JsonStack* js, const Squares src) {
    assert(dst != nullptr);
    size_t written = 0;

    written += CharSlice_WriteJsonStart(dst, js, '{');
    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            const Pos  pos   = {.row = i, .col = j};
            const auto piece = Squares_ConstAt(src, pos);

            if (!Piece_IsEmpty(piece)) {
                auto posSlice = CharSlice_Make(0, 64);
                CharSlice_WritePos(&posSlice, pos);

                written += CharSlice_WriteJsonKey(dst, js, posSlice);
                written += CharSlice_WritePieceAsJson(dst, js, piece);
            }
        }
    }
    written += CharSlice_WriteJsonEnd(dst, js);
    return written;
}
size_t CharSlice_WritePieceTypeArrAsJson(CharSlice* dst, JsonStack* js, const PieceTypes* src) {
    assert(dst != nullptr);
    assert(js != nullptr);
    assert(src != nullptr);

    size_t written = 0;
    written += CharSlice_WriteJsonStart(dst, js, '[');
    for (size_t i = 0; i < src->len; i++) {
        const auto piece      = TakenPieces_At(src, i);
        auto       pieceSlice = CharSlice_Make(0, 64);
        CharSlice_WritePieceType(&pieceSlice, piece);
        written += CharSlice_WriteJsonString(dst, js, pieceSlice);
    }
    written += CharSlice_WriteJsonEnd(dst, js);
    return written;
}
size_t CharSlice_WriteSideStateAsJson(CharSlice* dst, JsonStack* js, const SideState* src) {
    assert(dst != nullptr);
    assert(js != nullptr);
    assert(src != nullptr);

    size_t written = 0;
    written += CharSlice_WriteJsonStart(dst, js, '{');
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("king_castled"));
    written += CharSlice_WriteJsonBool(dst, js, src->hasKingCastled);
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("taken"));
    written += CharSlice_WritePieceTypeArrAsJson(dst, js, &src->taken);
    written += CharSlice_WriteJsonEnd(dst, js);
    return written;
}

bool Threat_IsValid(const Threat t) { return Pos_IsValid(t.pos) && PieceType_IsValid(t.pieceType); }

size_t Threats_Append(Threats* ts, const Threat t) {
    assert(ts != nullptr);
    assert(Threat_IsValid(t));

    if (ts->len >= THREATS_CAP) {
        return 0;
    }

    ts->items[ts->len++] = t;
    return 1;
}

void Board_PlacePieces(Board* dst) {
    assert(dst != nullptr);

    dst->squares[7][0] = (Piece){.type = PIECE_TYPE_ROOK, .side = SIDE_WHITE};
    dst->squares[7][7] = (Piece){.type = PIECE_TYPE_ROOK, .side = SIDE_WHITE};
    dst->squares[7][1] = (Piece){.type = PIECE_TYPE_KNIGHT, .side = SIDE_WHITE};
    dst->squares[7][6] = (Piece){.type = PIECE_TYPE_KNIGHT, .side = SIDE_WHITE};
    dst->squares[7][2] = (Piece){.type = PIECE_TYPE_BISHOP, .side = SIDE_WHITE};
    dst->squares[7][5] = (Piece){.type = PIECE_TYPE_BISHOP, .side = SIDE_WHITE};
    dst->squares[7][3] = (Piece){.type = PIECE_TYPE_QUEEN, .side = SIDE_WHITE};
    dst->squares[7][4] = (Piece){.type = PIECE_TYPE_KING, .side = SIDE_WHITE};

    dst->squares[0][0] = (Piece){.type = PIECE_TYPE_ROOK, .side = SIDE_BLACK};
    dst->squares[0][7] = (Piece){.type = PIECE_TYPE_ROOK, .side = SIDE_BLACK};
    dst->squares[0][1] = (Piece){.type = PIECE_TYPE_KNIGHT, .side = SIDE_BLACK};
    dst->squares[0][6] = (Piece){.type = PIECE_TYPE_KNIGHT, .side = SIDE_BLACK};
    dst->squares[0][2] = (Piece){.type = PIECE_TYPE_BISHOP, .side = SIDE_BLACK};
    dst->squares[0][5] = (Piece){.type = PIECE_TYPE_BISHOP, .side = SIDE_BLACK};
    dst->squares[0][3] = (Piece){.type = PIECE_TYPE_QUEEN, .side = SIDE_BLACK};
    dst->squares[0][4] = (Piece){.type = PIECE_TYPE_KING, .side = SIDE_BLACK};

    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        dst->squares[6][i] = (Piece){.type = PIECE_TYPE_PAWN, .side = SIDE_WHITE};
        dst->squares[1][i] = (Piece){.type = PIECE_TYPE_PAWN, .side = SIDE_BLACK};
    }

    for (size_t i = 2; i < 6; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            dst->squares[i][j].type = PIECE_TYPE_NONE;
            dst->squares[i][j].side = SIDE_NONE;
        }
    }
}

bool Board_Equals(const Board* a, const Board* b) {
    assert(a != nullptr);
    assert(b != nullptr);
    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            const auto pieceA = &a->squares[i][j];
            const auto pieceB = &b->squares[i][j];
            if (!Piece_Equals(pieceA, pieceB)) {
                return false;
            }
        }
    }

    return true;
}

BoardParseResult Board_Parse(Board* dst, const CharSlice src) {
    assert(dst != nullptr);

    size_t i      = 0;
    size_t offset = 0;
    while (i < BOARD_SIZE) {
        if (offset >= src.len) {
            return (BoardParseResult){
                .err    = BOARD_PARSE_ERR_TOO_SHORT,
                .offset = offset,
            };
        }

        const auto ch = CharSlice_At(&src, offset++);
        if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t') {
            continue;
        }

        Piece piece = {};
        if (!Piece_FromChar(&piece, ch)) {
            return (BoardParseResult){
                .err            = BOARD_PARSE_ERR_UNEXPECTED_CHAR,
                .offset         = offset,
                .unexpectedChar = ch,
            };
        }

        const Pos  pos = {.col = i % BOARD_SIDE_LEN, .row = i / BOARD_SIDE_LEN};
        const auto sq  = Squares_At(dst->squares, pos);
        *sq            = piece;
        i++;
    }

    return (BoardParseResult){.offset = offset};
}

bool Board_InterpretJson(Board* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    const auto t = JsonTokens_At(src->tokens, src->index);
    if (t->type != JSON_TOKEN_TYPE_OBJECT) {
        // TODO: Add proper error handling
        return false;
    }

    for (size_t i = 0; i < t->childrenCount; i++) {
        const auto keyToken = JsonTokens_At(src->tokens, ++(src->index));
        if (keyToken->childrenCount != 1) {
            return false;
        }
        const auto valueToken = JsonTokens_At(src->tokens, ++(src->index));
        const auto key        = JsonToken_View(keyToken, src->charSlice);

        if (CharSlice_Equals(key, CHAR_SLICE("next_move_side"))) {
            const auto value  = JsonToken_View(valueToken, src->charSlice);
            dst->nextMoveSide = Side_Parse(value);
        } else if (CharSlice_Equals(key, CHAR_SLICE("squares"))) {
            if (!Squares_InterpretJson(dst->squares, src)) {
                return false;
            }
        } else if (CharSlice_Equals(key, CHAR_SLICE("white"))) {
            if (!SideState_InterpretJson(&dst->white, src)) {
                return false;
            }
        } else if (CharSlice_Equals(key, CHAR_SLICE("black"))) {
            if (!SideState_InterpretJson(&dst->black, src)) {
                return false;
            }
        } else {
            // Skipping other keys
            src->index = JsonTokens_Skip(src->tokens, ++src->index);
        }
    }

    return true;
}

bool SideState_InterpretJson(SideState* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    const auto t = JsonTokens_At(src->tokens, src->index);
    if (t->type != JSON_TOKEN_TYPE_OBJECT) {
        // TODO: Add proper error handling
        return false;
    }

    for (size_t i = 0; i < t->childrenCount; i++) {
        const auto keyToken = JsonTokens_At(src->tokens, ++(src->index));
        if (keyToken->childrenCount != 1) {
            return false;
        }
        const auto valueToken = JsonTokens_At(src->tokens, ++(src->index));
        const auto key        = JsonToken_View(keyToken, src->charSlice);

        if (CharSlice_Equals(key, CHAR_SLICE("king_castled"))) {
            const auto value    = JsonToken_View(valueToken, src->charSlice);
            dst->hasKingCastled = CharSlice_Equals(value, CHAR_SLICE("true"));
        } else if (CharSlice_Equals(key, CHAR_SLICE("taken"))) {
            if (!PieceTypes_InterpretJson(&dst->taken, src)) {
                return false;
            }
        } else {
            // Skipping other keys
            src->index = JsonTokens_Skip(src->tokens, ++src->index);
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

bool Squares_InterpretJson(Squares dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    const auto t = JsonTokens_At(src->tokens, src->index);
    if (t->type != JSON_TOKEN_TYPE_OBJECT) {
        // TODO: Implement proper error handling
        return false;
    }

    if (t->childrenCount < 1) {
        return false;
    }

    for (size_t i = 0; i < t->childrenCount; i++) {
        const auto keyToken = JsonTokens_At(src->tokens, ++(src->index));
        if (keyToken->childrenCount < 1) {
            return false;
        }
        const auto valueToken = JsonTokens_At(src->tokens, ++(src->index));
        if (valueToken->type != JSON_TOKEN_TYPE_OBJECT) {
            return false;
        }

        const auto key = JsonToken_View(keyToken, src->charSlice);
        Pos        pos = {};
        const auto r1  = Pos_Parse(&pos, key);
        if (r1.err != POS_PARSE_ERR_OK) {
            return false;
        }

        const auto piece = Squares_At(dst, pos);
        //++src->index;
        const auto r2 = Piece_InterpretJson(piece, src);
        if (!r2) {
            return false;
        }
    }

    return true;
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
        case PIECE_TYPE_NONE:
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

void Board_Copy(Board* dst, const Board* src) {
    assert(dst != nullptr);
    assert(src != nullptr);
    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            dst->squares[i][j] = src->squares[i][j];
        }
    }
}

size_t CharSlice_WriteBoard(CharSlice* dst, const Board* b) {
    size_t written = 0;
    for (size_t i = 0; i < BOARD_SIDE_LEN; ++i) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; ++j) {
            const Pos  pos       = {.row = i, .col = j};
            const auto piece     = Squares_ConstAt(b->squares, pos);
            const char pieceChar = (piece->side == SIDE_WHITE) ? PieceType_ToUpperCaseChar(piece->type)
                                                               : PieceType_ToLowerCaseChar(piece->type);

            written += CharSlice_WriteChar(dst, pieceChar);
        }
        written += CharSlice_WriteChar(dst, '\n');
    }

    return written;
}

size_t CharSlice_WriteBoardAsJson(CharSlice* dst, JsonStack* js, const Board* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    auto nextMoveSideSlice = CharSlice_Make(0, 64);
    CharSlice_WriteSide(&nextMoveSideSlice, src->nextMoveSide);

    size_t written = 0;
    written += CharSlice_WriteJsonStart(dst, js, '{');
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("next_move_side"));
    written += CharSlice_WriteJsonString(dst, js, nextMoveSideSlice);
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("squares"));
    written += CharSlice_WriteSquaresAsJson(dst, js, src->squares);
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("black"));
    written += CharSlice_WriteSideStateAsJson(dst, js, &src->black);
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("white"));
    written += CharSlice_WriteSideStateAsJson(dst, js, &src->white);
    written += CharSlice_WriteJsonEnd(dst, js);
    return written;
}

size_t Threats_Collect(Threats* dst, const Board* b, const Pos p) {
    const auto piece = Squares_ConstAt(b->squares, p);
    if (Piece_IsEmpty(piece)) {
        return 0;
    }

    size_t appended = 0;
    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            const Pos  opponentPos = {.col = i, .row = j};
            const auto opponent    = Squares_ConstAt(b->squares, opponentPos);
            if (opponent->side == piece->side) {
                continue;
            }

            const auto result = Board_CheckMove(b, (Move){.from = opponentPos, .to = p});
            if (result.err == MOVE_ERR_OK) {
                appended += Threats_Append(dst, (Threat){.pos = opponentPos, .pieceType = opponent->type});
            }
        }
    }

    return appended;
}
