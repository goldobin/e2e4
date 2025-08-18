#ifndef CHAR_SLICE_H
#define CHAR_SLICE_H
#include <stddef.h>
#include <stdio.h>

constexpr size_t CHAR_SLICE_DIFF_MAX_SIZE = 32;
constexpr size_t CHAR_SLICE_DIFF_DP_SIZE  = CHAR_SLICE_DIFF_MAX_SIZE + 1;

typedef struct {
    char*  arr;
    size_t len;
    size_t cap;
} CharSlice;

CharSlice CharSlice_New(char* buffer, size_t len, size_t cap);
char      CharSlice_At(const CharSlice* s, size_t i);
CharSlice CharSlice_View(const CharSlice* s, size_t start, size_t end);
size_t    CharSlice_WriteCharAt(CharSlice* dst, size_t offset, char v);
size_t    CharSlice_WriteAt(CharSlice* dst, size_t offset, CharSlice other);
size_t    CharSlice_WriteChar(CharSlice* dst, char v);
size_t    CharSlice_Write(CharSlice* dst, CharSlice other);
size_t    CharSlice_WriteString(CharSlice* dst, const char* src);
bool      CharSlice_StartsWith(const CharSlice* s, CharSlice prefix);
size_t    CharSlice_NoDiffLen(CharSlice s1, CharSlice s2);
void      CharSlice_Diff(CharSlice* dst, CharSlice s1, CharSlice s2);
int       CharSlice_Cmp(CharSlice a, CharSlice b);
size_t    CharSlice_WriteF(CharSlice* dst, const char* format, ...);
size_t    CharSlice_ReadLine(CharSlice* dst, FILE* src, char separator);
size_t    CharSlice_ReadFile(CharSlice* dst, FILE* src);
bool      CharSlice_IsValid(const CharSlice* s);
bool      CharSlice_IsNullTerminated(const CharSlice* s);

size_t File_WriteCharSlice(FILE* dst, CharSlice src);

#define CharSlice_Make(len, cap) ((cap) > 0 ? CharSlice_New((char[cap]){}, len, cap) : CharSlice_New(nullptr, 0, 0));
#define CHAR_SLICE(buffer) \
    (sizeof(buffer) > 1 ? CharSlice_New(buffer, sizeof(buffer) - 1, sizeof(buffer) - 1) : CharSlice_New(nullptr, 0, 0))

#endif  // CHAR_SLICE_H
