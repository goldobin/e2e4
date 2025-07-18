//
// Created by Oleksandr Goldobin on 18/07/2025.
//

#ifndef JSW_H
#define JSW_H

#include "char_slice.h"

typedef enum {
    JSW_FIELD = 0,
    JSW_ARRAY,
    JSW_OBJECT,
} JswStateType;

typedef struct {
    JswStateType type;
    bool         needsComma;
} JswState;

typedef struct {
    JswState* arr;
    size_t    len;
    size_t    cap;
} JswStack;

typedef struct {
    JswStack  stack;
    CharSlice dst;
} Jsw;

JswState* JswStack_Push(JswStack* stack);
JswState* JswStack_Top(const JswStack* stack);
JswState  JswStack_Pull(JswStack* stack);

void Jsw_Field(Jsw* w, CharSlice key);
void Jsw_Object(Jsw* w);
void Jsw_Array(Jsw* w);
void Jsw_Null(Jsw* w);
void Jsw_Bool(Jsw* w, bool v);
void Jsw_Numeric(Jsw* w, double v);
void Jsw_String(Jsw* w, CharSlice v);
void Jsw_End(Jsw* w);

#endif  // JSW_H
