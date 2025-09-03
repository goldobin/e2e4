#include "char_slice.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "func.h"

CharSlice CharSlice_Wrap(char* buffer, const size_t len, const size_t cap) {
    assert(buffer != nullptr);
    assert(cap > 0);
    assert(len <= cap);

    return (CharSlice){
        .arr = buffer,
        .len = len,
        .cap = cap,
    };
}

bool CharSlice_Equals(const CharSlice a, const CharSlice b) {
    assert(CharSlice_IsValid(a));
    assert(CharSlice_IsValid(b));

    if (a.len != b.len) {
        return false;
    }
    for (size_t i = 0; i < a.len; i++) {
        if (a.arr[i] != b.arr[i]) {
            return false;
        }
    }

    return true;
}

char CharSlice_At(const CharSlice s, const size_t i) {
    assert(i < s.len);
    return s.arr[i];
}

CharSlice CharSlice_View(const CharSlice s, const size_t start, const size_t end) {
    assert(start <= end);
    assert(end <= s.len);
    const auto len = end - start;
    return (CharSlice){
        .arr = s.arr + start,
        .len = len,
        .cap = s.cap - start,
    };
}

size_t CharSlice_WriteCharAt(CharSlice* dst, const size_t i, const char v) {
    assert(dst != nullptr);
    if (i > dst->len || i >= dst->cap) {
        return 0;
    }
    dst->arr[i] = v;
    if (i == dst->len) {
        dst->len++;
    }
    return 1;
}

size_t CharSlice_WriteAt(CharSlice* dst, const size_t offset, const CharSlice other) {
    assert(dst != nullptr);
    if (offset > dst->len || offset >= dst->cap) {
        return 0;
    }

    const auto itemsToCopy = MinSizeT(dst->cap - offset, other.len);
    if (itemsToCopy == 0) {
        return 0;
    }

    memcpy(&dst->arr[offset], &other.arr[0], itemsToCopy);
    dst->len = offset + itemsToCopy;
    return itemsToCopy;
}

size_t CharSlice_WriteChar(CharSlice* dst, const char v) { return CharSlice_WriteCharAt(dst, dst->len, v); }

size_t CharSlice_Write(CharSlice* dst, const CharSlice other) {
    assert(dst != nullptr);
    return CharSlice_WriteAt(dst, dst->len, other);
}

size_t CharSlice_WriteString(CharSlice* dst, const char* src) {
    assert(dst != nullptr);
    assert(src != nullptr);
    assert(CharSlice_IsValid(*dst));

    const auto remaining = dst->cap - dst->len;
    if (remaining < 1) {
        return 0;
    }

    const auto len = strnlen(src, remaining);
    if (len < 1) {
        return 0;
    }

    memcpy(dst->arr + dst->len, src, len);
    dst->len = dst->len + len;
    return len;
}

bool CharSlice_StartsWith(const CharSlice s, const CharSlice prefix) {
    if (prefix.len > s.len) {
        return false;
    }

    return memcmp(s.arr, prefix.arr, prefix.len) == 0;
}

size_t CharSlice_ToString(char* dst, const size_t maxLen, const CharSlice slice) {
    assert(dst != nullptr);

    const auto dstLen = MinSizeT(slice.len, maxLen - 1);
    memcpy(dst, slice.arr, dstLen);
    dst[dstLen] = '\0';
    return dstLen;
}

size_t CharSlice_ReadLine(CharSlice* dst, FILE* src, const char separator) {
    assert(dst != nullptr);
    assert(src != nullptr);
    assert(CharSlice_IsValid(*dst));

    const auto remaining = (int)(dst->cap - dst->len);
    if (remaining < 1) {
        return 0;
    }

    const auto s = fgets(dst->arr + dst->len, remaining, src);
    if (s == nullptr) {
        return 0;
    }

    auto written = strnlen(s, remaining);
    if (written < 1) {
        return 0;
    }

    if (s[written - 1] == separator) {
        s[written - 1] = '\0';
        written--;
    }

    dst->len = dst->len + written;
    return written;
}

size_t CharSlice_ReadFile(CharSlice* dst, FILE* src) {
    assert(dst != nullptr);
    assert(src != nullptr);

    const auto remaining = dst->cap - dst->len;
    if (remaining < 1) {
        return 0;
    }

    const auto read = fread(dst->arr + dst->len, sizeof(char), remaining, src);
    dst->len        = dst->len + read;
    return read;
}

bool CharSlice_IsValid(const CharSlice s) {
    if (s.arr == nullptr && s.len == 0 && s.cap == 0) {
        return true;
    }
    return s.arr != nullptr && s.len <= s.cap;
}

bool CharSlice_IsNullTerminated(const CharSlice s) {
    assert(CharSlice_IsValid(s));
    if (s.len == s.cap) {
        return false;
    }

    return s.arr[s.len] == '\0';
}

size_t File_WriteCharSlice(FILE* dst, const CharSlice src) {
    assert(dst != nullptr);
    return fwrite(src.arr, sizeof(char), src.len, dst);
}

size_t CharSlice_WriteF(CharSlice* dst, const char* format, ...) {
    assert(dst != nullptr);
    assert(format != nullptr);
    assert(CharSlice_IsValid(*dst));

    const auto remaining = dst->cap - dst->len;

    va_list args;
    va_start(args, format);
    const auto result = vsnprintf(dst->arr + dst->len, remaining, format, args);
    va_end(args);

    if (result < 0) {
        return 0;
    }

    const auto written = MinSizeT(result, remaining - 1);
    dst->len           = dst->len + written;
    return written;
}

size_t CharSlice_NoDiffLen(const CharSlice s1, const CharSlice s2) {
    size_t i = 0;
    while (i < s1.len && i < s2.len && CharSlice_At(s1, i) == CharSlice_At(s2, i)) {
        i++;
    }

    return i;
}

void CharSlice_Diff(CharSlice* dst, const CharSlice s1, const CharSlice s2) {
    const auto dstRemCap = dst->cap - dst->len;
    if (dstRemCap < 5 + 1 + 2 + 1 + 2 + 1 + 5) {
        return;
    }

    const size_t contextHalfLen = (dstRemCap - (1 + 2 + 1 + 2 + 1)) / 2;
    const auto   noDiffLen      = CharSlice_NoDiffLen(s1, s2);
    if (s1.len == s2.len && noDiffLen == s1.len) {
        return;
    }

    const auto srcMinLen = MinSizeT(s1.len, s2.len);

    size_t srcViewOffset = 0;
    size_t srcViewLen    = noDiffLen;
    if (noDiffLen > contextHalfLen) {
        srcViewOffset = noDiffLen - contextHalfLen;
        srcViewLen    = contextHalfLen;
    }

    const auto srcView = CharSlice_View(s1, srcViewOffset, srcViewOffset + srcViewLen);
    CharSlice_Write(dst, srcView);

    if (srcViewLen - 3 > noDiffLen) {
        CharSlice_WriteAt(dst, 0, CHAR_SLICE("..."));
    }

    if (noDiffLen == srcMinLen) {
        CharSlice_WriteChar(dst, '{');

        if (s1.len < s2.len) {
            size_t diffViewLen = s2.len - s1.len;
            if (diffViewLen > contextHalfLen) {
                diffViewLen = contextHalfLen;
            }

            CharSlice_Write(dst, CHAR_SLICE("\\0|"));
            const auto diffView = CharSlice_View(s2, s1.len, s1.len + diffViewLen);
            CharSlice_Write(dst, diffView);
            CharSlice_WriteChar(dst, '}');
        } else {
            size_t diffViewLen = s1.len - s2.len;
            if (diffViewLen > contextHalfLen) {
                diffViewLen = contextHalfLen;
            }

            const auto diffView = CharSlice_View(s1, s2.len, s2.len + diffViewLen);
            CharSlice_Write(dst, diffView);
            CharSlice_Write(dst, CHAR_SLICE("|\\0}"));
        }
        return;
    }

    const auto s1DiffView    = CharSlice_View(s1, noDiffLen, s1.len);
    const auto s2DiffView    = CharSlice_View(s2, noDiffLen, s2.len);
    const auto s1ContextLen  = MinSizeT(contextHalfLen / 2, s1.len - noDiffLen);
    const auto s2ContextLen  = MinSizeT(contextHalfLen / 2, s2.len - noDiffLen);
    const auto s1ContextView = CharSlice_View(s1DiffView, 0, s1ContextLen);
    const auto s2ContextView = CharSlice_View(s2DiffView, 0, s2ContextLen);

    CharSlice_WriteChar(dst, '{');
    CharSlice_Write(dst, s1ContextView);

    if (s1DiffView.len - 3 > s1ContextView.len) {
        CharSlice_WriteAt(dst, dst->len - 3, CHAR_SLICE("..."));
    }

    CharSlice_WriteChar(dst, '|');
    CharSlice_Write(dst, s2ContextView);
    if (s2DiffView.len - 3 > s2ContextView.len) {
        CharSlice_WriteAt(dst, dst->len - 3, CHAR_SLICE("..."));
    }
    CharSlice_WriteChar(dst, '}');
}

int CharSlice_Cmp(const CharSlice a, const CharSlice b) {
    const auto minLen = MinSizeT(a.len, b.len);
    const auto result = strncmp(a.arr, b.arr, minLen);
    if (result != 0) {
        return result;
    }

    if (a.len == b.len) {
        return 0;
    }

    return a.len < b.len ? -1 : 1;
}
