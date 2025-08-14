#include "json_writer.h"

#include <assert.h>

JswState* JswStack_Push(JswStack* stack) {
    assert(stack != nullptr);
    assert(stack->cap - stack->len > 0);

    return &stack->arr[stack->len++];
}

JswState* JswStack_Top(const JswStack* stack) {
    assert(stack != nullptr);
    assert(stack->len > 0);
    return &stack->arr[stack->len - 1];
}

JswState JswStack_Pull(JswStack* stack) {
    assert(stack != nullptr);
    assert(stack->len > 0);
    return stack->arr[--stack->len];
}

void Jsw_Field(Jsw* w, const CharSlice key) {
    assert(w != nullptr);
    assert(CharSlice_IsValid(&key));
    assert(w->stack.len > 0);

    const auto cur = JswStack_Top(&w->stack);
    assert(cur != nullptr);
    assert(cur->type == JSW_OBJECT);

    if (cur->needsComma) {
        CharSlice_WriteChar(&w->dst, ',');
    }
    cur->needsComma = true;

    const auto next = JswStack_Push(&w->stack);
    // TODO: Check for nullptr and produce error
    *next = (JswState){JSW_FIELD, false};

    CharSlice_WriteChar(&w->dst, '\"');
    CharSlice_Write(&w->dst, key);
    CharSlice_WriteChar(&w->dst, '\"');
    CharSlice_WriteChar(&w->dst, ':');
}

void Jsw_Object(Jsw* w) {
    assert(w != nullptr);

    if (w->stack.len > 0) {
        const auto cur = JswStack_Top(&w->stack);
        assert(cur != nullptr);
        assert(cur->type == JSW_FIELD);
    }

    CharSlice_WriteChar(&w->dst, '{');
    const auto next = JswStack_Push(&w->stack);
    // TODO: Check for nullptr and produce error
    *next = (JswState){JSW_OBJECT, false};
}

void Jsw_Array(Jsw* w) {
    assert(w != nullptr);

    if (w->stack.len > 0) {
        const auto cur = JswStack_Top(&w->stack);
        assert(cur->type == JSW_FIELD);
    }

    CharSlice_WriteChar(&w->dst, '[');
    const auto next = JswStack_Push(&w->stack);
    // TODO: Check for nullptr and produce error
    *next = (JswState){JSW_ARRAY, false};
}

void Jsw_BeforePrimitive(Jsw* w) {
    assert(w != nullptr);
    assert(w->stack.len > 0);

    const auto cur = JswStack_Top(&w->stack);
    switch (cur->type) {
        case JSW_FIELD:
            JswStack_Pull(&w->stack);
            cur->needsComma = true;
            break;
        case JSW_ARRAY:
            if (cur->needsComma) {
                CharSlice_WriteChar(&w->dst, ',');
            }
            cur->needsComma = true;
            break;
        default:
            assert(false);
    }
}

void Jsw_Null(Jsw* w) {
    Jsw_BeforePrimitive(w);
    CharSlice_Write(&w->dst, CHAR_SLICE("null"));
}

void Jsw_Bool(Jsw* w, const bool v) {
    assert(w != nullptr);
    Jsw_BeforePrimitive(w);
    CharSlice_Write(&w->dst, v ? CHAR_SLICE("true") : CHAR_SLICE("false"));
}

void Jsw_Numeric(Jsw* w, double v) {
    assert(w != nullptr);
    Jsw_BeforePrimitive(w);
    CharSlice_WriteF(&w->dst, "%f", v);
}

void Jsw_String(Jsw* w, const CharSlice v) {
    assert(w != nullptr);
    Jsw_BeforePrimitive(w);
    CharSlice_WriteChar(&w->dst, '\"');
    CharSlice_Write(&w->dst, v);
    CharSlice_WriteChar(&w->dst, '\"');
}

void Jsw_End(Jsw* w) {
    assert(w != nullptr);
    assert(w->stack.len > 0);

    const auto pulled = JswStack_Pull(&w->stack);
    assert(pulled.type == JSW_OBJECT || pulled.type == JSW_ARRAY);

    switch (pulled.type) {
        case JSW_OBJECT:
            CharSlice_WriteChar(&w->dst, '}');
            break;
        case JSW_ARRAY:
            CharSlice_WriteChar(&w->dst, ']');
            break;
        default:
            unreachable();
    }

    if (w->stack.len > 0 && JswStack_Top(&w->stack)->type == JSW_FIELD) {
        JswStack_Pull(&w->stack);
    }
}
