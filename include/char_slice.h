#ifndef CHAR_SLICE_H
#define CHAR_SLICE_H
#include <stddef.h>
#include <stdio.h>

#include "arena.h"

constexpr size_t CHAR_SLICE_DIFF_MAX_SIZE = 32;
constexpr size_t CHAR_SLICE_DIFF_DP_SIZE  = CHAR_SLICE_DIFF_MAX_SIZE + 1;

typedef struct {
    const char*  arr;
    const size_t len;
} Str;

typedef struct {
    char*  arr;
    size_t len;
    size_t cap;
} CharSlice;

char   Str_At(Str s, size_t i);
Str    Str_View(Str s, size_t start, size_t end);
int    Str_Cmp(Str a, Str b);
bool   Str_Equals(Str a, Str b);
bool   Str_StartsWith(Str s, Str prefix);
bool   Str_IsValid(Str s);
size_t Str_NoDiffLen(Str s1, Str s2);

bool   CharSlice_Alloc(CharSlice* dst, size_t cap, Arena* src);
bool   CharSlice_Equals(CharSlice a, CharSlice b);
bool   CharSlice_EqualsStr(CharSlice a, Str b);
char   CharSlice_At(CharSlice s, size_t i);
Str    CharSlice_View(CharSlice s, size_t start, size_t end);
Str    CharSlice_ToStr(CharSlice s);
size_t CharSlice_WriteCharAt(CharSlice* dst, size_t i, char v);
size_t CharSlice_WriteAt(CharSlice* dst, size_t offset, CharSlice src);
size_t CharSlice_WriteStrAt(CharSlice* dst, size_t offset, Str src);
size_t CharSlice_WriteChar(CharSlice* dst, char v);
size_t CharSlice_Write(CharSlice* dst, CharSlice other);
size_t CharSlice_WriteStr(CharSlice* dst, Str src);
size_t CharSlice_WriteZeroStr(CharSlice* dst, const char* src);
void   CharSlice_WriteDiff(CharSlice* dst, Str s1, Str s2);
bool   CharSlice_StartsWith(CharSlice s, CharSlice prefix);

int    CharSlice_Cmp(CharSlice a, CharSlice b);
size_t CharSlice_WriteF(CharSlice* dst, const char* format, ...);
size_t CharSlice_ReadLine(CharSlice* dst, FILE* src, char separator);
size_t CharSlice_ReadFile(CharSlice* dst, FILE* src);
bool   CharSlice_IsValid(CharSlice s);

size_t File_WriteCharSlice(FILE* dst, CharSlice src);

#define CharSlice_OnStack(len1, cap1)                                         \
    ((cap1) >= 0 && (len1) >= 0 && (len1) <= (cap1)                           \
         ? (CharSlice){.arr = (char[(cap1)]){}, .len = (len1), .cap = (cap1)} \
         : (CharSlice){})

#define CharSlice_OnStack1(len1, cap1)                                  \
    ((struct {                                                          \
         int z;                                                         \
         static_assert((cap1) >= 0 && (len1) >= 0 && (len1) <= (cap1)); \
     }){1},                                                             \
     (CharSlice){.arr = (char[(cap1)]){}, .len = (len1), .cap = (cap1)})

#define CHAR_SLICE(buffer) (sizeof(buffer) > 1 ? (Str){.arr = (buffer), .len = (sizeof(buffer) - 1)} : (Str){})

#endif  // CHAR_SLICE_H
