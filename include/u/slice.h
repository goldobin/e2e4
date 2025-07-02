#ifndef U_STRINGS_H
#define U_STRINGS_H
#include <stddef.h>

constexpr size_t CHAR_SLICE_DIFF_MAX_SIZE = 32;
constexpr size_t CHAR_SLICE_DIFF_DP_SIZE  = CHAR_SLICE_DIFF_MAX_SIZE + 1;

typedef struct {
    char*  chars;
    size_t len;
    size_t cap;
} CharSlice;

CharSlice CharSlice_New(char* buffer, size_t len, size_t cap);
char      CharSlice_At(const CharSlice* s, size_t i);
CharSlice CharSlice_View(const CharSlice* s, size_t start, size_t end);
size_t    CharSlice_WriteOneAt(CharSlice* s, size_t offset, char ch);
size_t    CharSlice_WriteAt(CharSlice* s, size_t offset, CharSlice other);
size_t    CharSlice_WriteOne(CharSlice* s, char ch);
size_t    CharSlice_Write(CharSlice* s, CharSlice other);
void      CharSlice_WriteDiff(CharSlice* dst, CharSlice s1, CharSlice s2);
size_t    CharSlice_ToString(char* dst, size_t maxLen, CharSlice slice);

#define CharSlice_Wrap(buffer)       CharSlice_New(buffer, sizeof(buffer), sizeof(buffer))
#define CharSlice_WrapEmpty(buffer)  CharSlice_New(buffer, 0, sizeof(buffer))
#define CharSlice_WrapString(buffer) CharSlice_New(buffer, sizeof(buffer) - 1, sizeof(buffer) - 1)

#define CharSlice_WriteStringAt(s, offset, str) CharSlice_WriteAt(s, offset, CharSlice_WrapString(str))
#define CharSlice_WriteString(s, str)           CharSlice_Write(s, CharSlice_WrapString(str))

#endif  // U_STRINGS_H
