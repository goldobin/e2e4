#ifndef CHAR_SLICE_H
#define CHAR_SLICE_H
#include <stddef.h>
#include <stdio.h>

#include "arena.h"

constexpr size_t CHAR_BUFF_DIFF_MAX_SIZE = 32;

typedef struct {
    const char*  arr;
    const size_t len;
} Str;

typedef struct {
    char*  arr;
    size_t len;
    size_t cap;
} CharBuff;

Str    Str_FromCStr(const char* s, size_t maxLen);
char   Str_At(Str s, size_t i);
Str    Str_View(Str s, size_t start, size_t end);
int    Str_Cmp(Str a, Str b);
bool   Str_Equals(Str a, Str b);
bool   Str_StartsWith(Str s, Str prefix);
bool   Str_IsValid(Str s);
size_t Str_NoDiffLen(Str s1, Str s2);

bool   CharBuff_Alloc(CharBuff* dst, size_t cap, Arena* src);
bool   CharBuff_IsValid(CharBuff s);
bool   CharBuff_Equals(CharBuff a, CharBuff b);
bool   CharBuff_EqualsStr(CharBuff a, Str b);
char   CharBuff_At(CharBuff s, size_t i);
Str    CharBuff_View(CharBuff s, size_t start, size_t end);
Str    CharBuff_ToStr(CharBuff s);
bool   CharBuff_StartsWith(CharBuff s, CharBuff prefix);
int    CharBuff_Cmp(CharBuff a, CharBuff b);
size_t CharBuff_WriteCharAt(CharBuff* dst, size_t i, char v);
size_t CharBuff_WriteAt(CharBuff* dst, size_t offset, CharBuff src);
size_t CharBuff_WriteStrAt(CharBuff* dst, size_t offset, Str src);
size_t CharBuff_WriteChar(CharBuff* dst, char v);
size_t CharBuff_Write(CharBuff* dst, CharBuff other);
size_t CharBuff_WriteStr(CharBuff* dst, Str src);
void   CharBuff_WriteDiff(CharBuff* dst, Str s1, Str s2);
size_t CharBuff_WriteF(CharBuff* dst, const char* format, ...);
size_t CharBuff_WriteFile(CharBuff* dst, FILE* src);
size_t CharBuff_WriteLineFromFile(CharBuff* dst, FILE* src, char separator);

size_t File_WriteCharBuff(FILE* dst, CharBuff src);

#define STR(buffer) (sizeof(buffer) > 1 ? (Str){.arr = (buffer), .len = (sizeof(buffer) - 1)} : (Str){})

#define CharBuff_OnStack(len1, cap1)                                    \
    ((struct {                                                          \
         int z;                                                         \
         static_assert((cap1) >= 0 && (len1) >= 0 && (len1) <= (cap1)); \
     }){1},                                                             \
     (CharBuff){.arr = (char[(cap1)]){}, .len = (len1), .cap = (cap1)})

#endif  // CHAR_SLICE_H
