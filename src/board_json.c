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
        if (CharSlice_Equals(key, CHAR_SLICE("side"))) {
            if (JsonSource_Type(src) != JSON_TYPE_STRING) {
                return false;
            }
            dst->side = Side_Parse(JsonSource_Value(src));
        } else if (CharSlice_Equals(key, CHAR_SLICE("type"))) {
            if (JsonSource_Type(src) != JSON_TYPE_STRING) {
                return false;
            }
            dst->type = PieceType_Parse(JsonSource_Value(src));
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

        const auto pieceType = PieceType_Parse(JsonSource_Value(src));
        PieceTypes_UpdateAt(dst, i, pieceType);
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
        if (CharSlice_Equals(key, CHAR_SLICE("next_move_side"))) {
            if (JsonSource_Type(src) != JSON_TYPE_STRING) {
                return false;
            }
            dst->side = Side_Parse(JsonSource_Value(src));
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
            JsonSource_Skip(src);
        }
    }

    return true;
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
        if (CharSlice_Equals(key, CHAR_SLICE("king_castled"))) {
            if (JsonSource_Type(src) != JSON_TYPE_BOOL) {
                return false;
            }
            dst->hasKingCastled = JsonSource_BoolValue(src);
        } else if (CharSlice_Equals(key, CHAR_SLICE("taken"))) {
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

size_t CharSlice_WritePieceAsJson(CharSlice* dst, JsonStack* js, const Piece p) {
    assert(dst != nullptr);
    assert(js != nullptr);

    auto pieceTypeSlice = CharSlice_Make(0, 32);
    auto sideSlice      = CharSlice_Make(0, 32);

    CharSlice_WritePieceType(&pieceTypeSlice, p.type);
    CharSlice_WriteSide(&sideSlice, p.side);

    size_t written = 0;
    written += CharSlice_WriteJsonStart(dst, js, '{');
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("type"));
    written += CharSlice_WriteJsonString(dst, js, pieceTypeSlice);
    written += CharSlice_WriteJsonKey(dst, js, CHAR_SLICE("side"));
    written += CharSlice_WriteJsonString(dst, js, sideSlice);
    written += CharSlice_WriteJsonEnd(dst, js);
    return written;
}

size_t CharSlice_WriteSquaresAsJson(CharSlice* dst, JsonStack* js, const Squares src) {
    assert(dst != nullptr);
    size_t written = 0;

    written += CharSlice_WriteJsonStart(dst, js, '{');
    for (size_t i = 0; i < BOARD_SIDE_LEN; i++) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; j++) {
            const Pos  pos   = {.row = i, .col = j};
            const auto piece = Squares_At(src, pos);

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

size_t CharSlice_WritePieceTypesAsJson(CharSlice* dst, JsonStack* js, const PieceTypes src) {
    assert(dst != nullptr);
    assert(js != nullptr);

    size_t written = 0;
    written += CharSlice_WriteJsonStart(dst, js, '[');
    for (size_t i = 0; i < src.len; i++) {
        const auto t          = PieceTypes_At(src, i);
        auto       pieceSlice = CharSlice_Make(0, 64);
        CharSlice_WritePieceType(&pieceSlice, t);
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
    written += CharSlice_WritePieceTypesAsJson(dst, js, src->taken);
    written += CharSlice_WriteJsonEnd(dst, js);
    return written;
}

size_t CharSlice_WriteBoardAsJson(CharSlice* dst, JsonStack* js, const Board* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    auto nextMoveSideSlice = CharSlice_Make(0, 64);
    CharSlice_WriteSide(&nextMoveSideSlice, src->side);

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
