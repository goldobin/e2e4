#include "board_json.h"

#include <assert.h>

#include "board_repr.h"

// Interpret

bool Piece_InterpretJson(Piece* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    if (JsonSource_Type(src) != JSON_TYPE_OBJECT) {
        return false;
    }

    const auto keyCount = JsonSource_ChildrenCount(src);

    if (keyCount < 2) {
        return false;
    }

    for (size_t i = 0; i < keyCount; ++i) {
        JsonSource_Next(src);
        if (JsonSource_Type(src) != JSON_TYPE_KEY) {
            return false;
        }

        const auto key = JsonSource_Value(src);
        JsonSource_Next(src);
        if (Str_Equals(key, STR("side"))) {
            if (JsonSource_Type(src) != JSON_TYPE_STRING) {
                return false;
            }
            auto r = Side_Parse(&dst->side, JsonSource_Value(src));
            if (r.err != SIDE_PARSE_ERR_OK) {
                return false;
            }
        } else if (Str_Equals(key, STR("type"))) {
            if (JsonSource_Type(src) != JSON_TYPE_STRING) {
                return false;
            }
            const auto r = PieceType_Parse(&dst->type, JsonSource_Value(src));
            if (r.err != PIECE_TYPE_PARSE_ERR_OK) {
                return false;
            }
        } else {
            JsonSource_Skip(src);
        }
    }

    return true;
}

bool PieceTypes_InterpretJson(PieceTypes* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    if (JsonSource_Type(src) != JSON_TYPE_ARRAY) {
        return false;
    }

    const auto arraySize = JsonSource_ChildrenCount(src);
    if (!PieceTypes_Resize(dst, arraySize)) {
        return false;
    }

    for (size_t i = 0; i < arraySize; i++) {
        JsonSource_Next(src);
        if (JsonSource_Type(src) != JSON_TYPE_STRING) {
            return false;
        }

        const auto r = PieceType_Parse(PieceTypes_AtRef(dst, i), JsonSource_Value(src));
        if (r.err != PIECE_TYPE_PARSE_ERR_OK) {
            return false;
        }
    }

    return true;
}

bool Board_InterpretJson(Board* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    if (JsonSource_Type(src) != JSON_TYPE_OBJECT) {
        return false;
    }

    const auto keyCount = JsonSource_ChildrenCount(src);

    for (size_t i = 0; i < keyCount; i++) {
        JsonSource_Next(src);
        if (JsonSource_Type(src) != JSON_TYPE_KEY) {
            return false;
        }
        const auto key = JsonSource_Value(src);
        JsonSource_Next(src);
        if (Str_Equals(key, STR("state"))) {
            if (JsonSource_Type(src) != JSON_TYPE_STRING) {
                return false;
            }
            const auto r = BoardState_Parse(&dst->state, JsonSource_Value(src));
            if (r.err != BOARD_STATE_PARSE_ERR_OK) {
                return false;
            };
        } else if (Str_Equals(key, STR("next_move_side"))) {
            if (JsonSource_Type(src) != JSON_TYPE_STRING) {
                return false;
            }
            const auto r = Side_Parse(&dst->side, JsonSource_Value(src));
            if (r.err != SIDE_PARSE_ERR_OK) {
                return false;
            }
        } else if (Str_Equals(key, STR("squares"))) {
            if (!Squares_InterpretJson(dst->squares, src)) {
                return false;
            }
        } else if (Str_Equals(key, STR("white"))) {
            if (!SideState_InterpretJson(&dst->white, src)) {
                return false;
            }
        } else if (Str_Equals(key, STR("black"))) {
            if (!SideState_InterpretJson(&dst->black, src)) {
                return false;
            }
        } else if (Str_Equals(key, STR("steps"))) {
            if (!Steps_InterpretJson(&dst->steps, src)) {
                return false;
            }
        } else {
            // Skipping other keys
            JsonSource_Skip(src);
        }
    }

    return true;
}
bool Step_InterpretJson(Step* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    if (JsonSource_Type(src) != JSON_TYPE_OBJECT) {
        return false;
    }

    const auto keyCount = JsonSource_ChildrenCount(src);
    for (size_t i = 0; i < keyCount; i++) {
        JsonSource_Next(src);
        if (JsonSource_Type(src) != JSON_TYPE_KEY) {
            return false;
        }

        const auto key = JsonSource_Value(src);
        JsonSource_Next(src);
        if (Str_Equals(key, STR("piece"))) {
            if (!Piece_InterpretJson(&dst->pice, src)) {
                return false;
            };
        } else if (Str_Equals(key, STR("move"))) {
            if (JsonSource_Type(src) != JSON_TYPE_STRING) {
                return false;
            }
            const auto v = JsonSource_Value(src);
            const auto r = Move_Parse(&dst->move, v);
            if (r.err != MOVE_PARSE_ERR_OK) {
                return false;
            }
        } else if (Str_Equals(key, STR("time"))) {
            if (JsonSource_Type(src) != JSON_TYPE_STRING) {
                return false;
            }

            const auto v = JsonSource_Value(src);
            if (!Time_ParseISO8601(&dst->time, v)) {
                return false;
            };
        } else {
            // Skipping other keys
            JsonSource_Skip(src);
        }
    }
    return true;
}

bool Steps_InterpretJson(Steps* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    if (JsonSource_Type(src) != JSON_TYPE_ARRAY) {
        return false;
    }

    const auto arrLen = JsonSource_ChildrenCount(src);
    if (!Steps_Resize(dst, arrLen)) {
        return false;
    }

    for (size_t i = 0; i < arrLen; i++) {
        JsonSource_Next(src);
        if (!Step_InterpretJson(Steps_AtRef(dst, i), src)) {
            return false;
        }
    }
    return true;
}

size_t CharBuff_WriteMoveAsJson(CharBuff* dst, JsonStack* js, Move m) {
    assert(dst != nullptr);
    assert(js != nullptr);

    auto buff = CharBuff_OnStack(0, 8);
    CharBuff_WriteMove(&buff, m);
    return CharBuff_WriteJsonStr(dst, js, CharBuff_ToStr(buff));
}

size_t CharBuff_WriteSideAsJson(CharBuff* dst, JsonStack* js, Side s) {
    switch (s) {
        case SIDE_WHITE:
            return CharBuff_WriteJsonStr(dst, js, STR("WHITE"));
        case SIDE_BLACK:
            return CharBuff_WriteJsonStr(dst, js, STR("BLACK"));
        default:
            return CharBuff_WriteJsonNull(dst, js);
    }
}

bool SideState_InterpretJson(SideState* dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    if (JsonSource_Type(src) != JSON_TYPE_OBJECT) {
        return false;
    }

    const auto childrenCount = JsonSource_ChildrenCount(src);
    for (size_t i = 0; i < childrenCount; i++) {
        JsonSource_Next(src);
        if (JsonSource_Type(src) != JSON_TYPE_KEY) {
            return false;
        }

        const auto key = JsonSource_Value(src);
        JsonSource_Next(src);
        if (Str_Equals(key, STR("king_castled"))) {
            if (JsonSource_Type(src) != JSON_TYPE_BOOL) {
                return false;
            }
            dst->hasKingCastled = JsonSource_BoolValue(src);
        } else if (Str_Equals(key, STR("taken"))) {
            if (!PieceTypes_InterpretJson(&dst->taken, src)) {
                return false;
            }
        } else {
            // Skipping other keys
            JsonSource_Skip(src);
        }
    }

    return true;
}

bool Squares_InterpretJson(Squares dst, JsonSource* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    if (JsonSource_Type(src) != JSON_TYPE_OBJECT) {
        return false;
    }

    const auto keyCount = JsonSource_ChildrenCount(src);
    for (size_t i = 0; i < keyCount; i++) {
        JsonSource_Next(src);
        if (JsonSource_Type(src) != JSON_TYPE_KEY) {
            return false;
        }

        Pos        pos            = {};
        const auto posParseResult = Pos_Parse(&pos, JsonSource_Value(src));
        if (posParseResult.err != POS_PARSE_ERR_OK) {
            return false;
        }

        JsonSource_Next(src);
        Piece piece = {};
        if (!Piece_InterpretJson(&piece, src)) {
            return false;
        };

        Squares_UpdateAt(dst, pos, piece);
    }

    return true;
}

// Write

size_t CharBuff_WritePieceAsJson(CharBuff* dst, JsonStack* js, const Piece p) {
    assert(dst != nullptr);
    assert(js != nullptr);

    size_t written = 0;
    written += CharBuff_WriteJsonStart(dst, js, '{');
    written += CharBuff_WriteJsonKey(dst, js, STR("type"));
    written += CharBuff_WritePieceTypeAsJson(dst, js, p.type);
    written += CharBuff_WriteJsonKey(dst, js, STR("side"));
    written += CharBuff_WriteSideAsJson(dst, js, p.side);
    written += CharBuff_WriteJsonEnd(dst, js);
    return written;
}
size_t CharBuff_WritePieceTypeAsJson(CharBuff* dst, JsonStack* js, const PieceType t) {
    assert(dst != nullptr);
    assert(js != nullptr);

    switch (t) {
        case PIECE_TYPE_PAWN:
            return CharBuff_WriteJsonStr(dst, js, STR("PAWN"));
        case PIECE_TYPE_ROOK:
            return CharBuff_WriteJsonStr(dst, js, STR("ROOK"));
        case PIECE_TYPE_KNIGHT:
            return CharBuff_WriteJsonStr(dst, js, STR("KNIGHT"));
        case PIECE_TYPE_BISHOP:
            return CharBuff_WriteJsonStr(dst, js, STR("BISHOP"));
        case PIECE_TYPE_QUEEN:
            return CharBuff_WriteJsonStr(dst, js, STR("QUEEN"));
        case PIECE_TYPE_KING:
            return CharBuff_WriteJsonStr(dst, js, STR("KING"));
        default:
            return CharBuff_WriteJsonNull(dst, js);
    }
}

size_t CharBuff_WriteSquaresAsJson(CharBuff* dst, JsonStack* js, const Squares src) {
    assert(dst != nullptr);
    size_t written = 0;

    written += CharBuff_WriteJsonStart(dst, js, '{');
    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            const Pos  pos   = {.row = i, .col = j};
            const auto piece = Squares_At(src, pos);
            if (!Piece_IsEmpty(piece)) {
                auto posBuff = CharBuff_OnStack(0, 64);
                CharBuff_WritePos(&posBuff, pos);
                written += CharBuff_WriteJsonKey(dst, js, CharBuff_ToStr(posBuff));
                written += CharBuff_WritePieceAsJson(dst, js, piece);
            }
        }
    }
    written += CharBuff_WriteJsonEnd(dst, js);
    return written;
}

size_t CharBuff_WritePieceTypesAsJson(CharBuff* dst, JsonStack* js, const PieceTypes* src) {
    assert(dst != nullptr);
    assert(js != nullptr);
    assert(src != nullptr);

    size_t written = 0;
    written += CharBuff_WriteJsonStart(dst, js, '[');
    for (size_t i = 0; i < src->len; i++) {
        const auto t = PieceTypes_At(src, i);
        written += CharBuff_WritePieceTypeAsJson(dst, js, *t);
    }
    written += CharBuff_WriteJsonEnd(dst, js);
    return written;
}

size_t CharBuff_WriteSideStateAsJson(CharBuff* dst, JsonStack* js, const SideState* src) {
    assert(dst != nullptr);
    assert(js != nullptr);
    assert(src != nullptr);

    if (src == nullptr) {
        return CharBuff_WriteJsonNull(dst, js);
    }

    size_t written = 0;
    written += CharBuff_WriteJsonStart(dst, js, '{');
    written += CharBuff_WriteJsonKey(dst, js, STR("king_castled"));
    written += CharBuff_WriteJsonBool(dst, js, src->hasKingCastled);
    written += CharBuff_WriteJsonKey(dst, js, STR("taken"));
    written += CharBuff_WritePieceTypesAsJson(dst, js, &src->taken);
    written += CharBuff_WriteJsonEnd(dst, js);
    return written;
}

size_t CharBuff_WriteBoardAsJson(CharBuff* dst, JsonStack* js, const Board* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    if (src == nullptr) {
        return CharBuff_WriteJsonNull(dst, js);
    }

    size_t written = 0;
    written += CharBuff_WriteJsonStart(dst, js, '{');
    written += CharBuff_WriteJsonKey(dst, js, STR("state"));
    written += CharBuff_WriteBoardStateAsJson(dst, js, src->state);
    written += CharBuff_WriteJsonKey(dst, js, STR("next_move_side"));
    written += CharBuff_WriteSideAsJson(dst, js, src->side);
    written += CharBuff_WriteJsonKey(dst, js, STR("squares"));
    written += CharBuff_WriteSquaresAsJson(dst, js, src->squares);
    written += CharBuff_WriteJsonKey(dst, js, STR("black"));
    written += CharBuff_WriteSideStateAsJson(dst, js, &src->black);
    written += CharBuff_WriteJsonKey(dst, js, STR("white"));
    written += CharBuff_WriteSideStateAsJson(dst, js, &src->white);
    written += CharBuff_WriteJsonKey(dst, js, STR("steps"));
    written += CharBuff_WriteStepsAsJson(dst, js, &src->steps);
    written += CharBuff_WriteJsonEnd(dst, js);
    return written;
}
size_t CharBuff_WriteStepAsJson(CharBuff* dst, JsonStack* js, const Step* src) {
    assert(dst != nullptr);
    assert(js != nullptr);

    if (src == nullptr) {
        return CharBuff_WriteJsonNull(dst, js);
    }

    size_t written = 0;
    written += CharBuff_WriteJsonStart(dst, js, '{');
    written += CharBuff_WriteJsonKey(dst, js, STR("move"));
    written += CharBuff_WriteMoveAsJson(dst, js, src->move);
    written += CharBuff_WriteJsonKey(dst, js, STR("piece"));
    written += CharBuff_WritePieceAsJson(dst, js, src->pice);
    written += CharBuff_WriteJsonKey(dst, js, STR("time"));
    written += CharBuff_WriteTimeAsJson(dst, js, src->time);
    written += CharBuff_WriteJsonEnd(dst, js);

    return written;
}
size_t CharBuff_WriteStepsAsJson(CharBuff* dst, JsonStack* js, const Steps* src) {
    assert(dst != nullptr);
    assert(js != nullptr);
    assert(src != nullptr);

    if (src == nullptr) {
        return CharBuff_WriteJsonNull(dst, js);
    }

    size_t written = 0;
    written += CharBuff_WriteJsonStart(dst, js, '[');
    for (size_t i = 0; i < src->len; i++) {
        written += CharBuff_WriteStepAsJson(dst, js, Steps_At(src, i));
    }
    written += CharBuff_WriteJsonEnd(dst, js);
    return written;
}
size_t CharBuff_WriteBoardStateAsJson(CharBuff* dst, JsonStack* js, BoardState s) {
    assert(dst != nullptr);
    assert(js != nullptr);

    switch (s) {
        case BOARD_STATE_IN_PROGRESS:
            return CharBuff_WriteJsonStr(dst, js, STR("IN_PROGRESS"));
        case BOARD_STATE_CHECKMATE:
            return CharBuff_WriteJsonStr(dst, js, STR("CHECKMATE"));
        case BOARD_STATE_STALEMATE:
            return CharBuff_WriteJsonStr(dst, js, STR("STALEMATE"));
        default:
            return CharBuff_WriteJsonNull(dst, js);
    }
}
