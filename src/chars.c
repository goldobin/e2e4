#include "chars.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "func.h"

char Str_At(const Str s, size_t i) {
    assert(i < s.len);
    return s.arr[i];
}
Str Str_View(const Str s, const size_t start, const size_t end) {
    assert(start <= end);
    assert(end <= s.len);
    const auto len = end - start;
    return (Str){
        .arr = s.arr + start,
        .len = len,
    };
}

bool Str_Equals(const Str a, const Str b) {
    if (a.len != b.len) {
        return false;
    }
    return memcmp(a.arr, b.arr, a.len) == 0;
}
bool Str_StartsWith(const Str s, const Str prefix) {
    if (prefix.len > s.len) {
        return false;
    }
    return memcmp(s.arr, prefix.arr, prefix.len) == 0;
}
bool Str_IsValid(const Str s) {
    if (s.arr == nullptr && s.len == 0) {
        return true;
    }
    return s.arr != nullptr;
}
bool CharBuff_Alloc(CharBuff* dst, size_t cap, Arena* src) {
    assert(dst != nullptr);
    assert(src != nullptr);
    assert(cap > 0);

    char* buff = Arena_Alloc(src, cap);
    if (buff == nullptr) {
        return false;
    }

    dst->arr = buff;
    dst->len = 0;
    dst->cap = cap;
    return true;
}

bool CharBuff_Equals(const CharBuff a, const CharBuff b) {
    assert(CharBuff_IsValid(a));
    assert(CharBuff_IsValid(b));

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
bool CharBuff_EqualsStr(const CharBuff a, const Str b) {
    assert(CharBuff_IsValid(a));

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

char CharBuff_At(const CharBuff s, const size_t i) {
    assert(i < s.len);
    return s.arr[i];
}

Str CharBuff_View(const CharBuff s, const size_t start, const size_t end) {
    assert(start <= end);
    assert(end <= s.len);
    const auto len = end - start;
    return (Str){
        .arr = s.arr + start,
        .len = len,
    };
}
Str CharBuff_ToStr(const CharBuff s) { return CharBuff_View(s, 0, s.len); }

size_t CharBuff_WriteCharAt(CharBuff* dst, const size_t i, const char v) {
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

size_t CharBuff_WriteAt(CharBuff* dst, const size_t offset, const CharBuff src) {
    assert(dst != nullptr);
    if (offset > dst->len || offset >= dst->cap) {
        return 0;
    }

    const auto itemsToCopy = MinSizeT(dst->cap - offset, src.len);
    if (itemsToCopy == 0) {
        return 0;
    }

    memcpy(&dst->arr[offset], &src.arr[0], itemsToCopy);
    dst->len = offset + itemsToCopy;
    return itemsToCopy;
}

size_t CharBuff_WriteStrAt(CharBuff* dst, const size_t offset, const Str src) {
    assert(dst != nullptr);
    if (offset > dst->len || offset >= dst->cap) {
        return 0;
    }

    const auto itemsToCopy = MinSizeT(dst->cap - offset, src.len);
    if (itemsToCopy == 0) {
        return 0;
    }

    memcpy(&dst->arr[offset], &src.arr[0], itemsToCopy);
    dst->len = offset + itemsToCopy;
    return itemsToCopy;
}

size_t CharBuff_WriteChar(CharBuff* dst, const char v) { return CharBuff_WriteCharAt(dst, dst->len, v); }

size_t CharBuff_Write(CharBuff* dst, const CharBuff other) {
    assert(dst != nullptr);
    return CharBuff_WriteAt(dst, dst->len, other);
}
size_t CharBuff_WriteStr(CharBuff* dst, const Str src) {
    assert(dst != nullptr);

    if (src.len == 0 || dst->len == dst->cap) {
        return 0;
    }

    const size_t remaining = dst->cap - dst->len;
    if (remaining < 1) {
        return 0;
    }

    const size_t len = MinSizeT(remaining, src.len);
    memcpy(dst->arr + dst->len, src.arr, len);
    dst->len = dst->len + len;
    return len;
}

size_t CharBuff_WriteZeroStr(CharBuff* dst, const char* src) {
    assert(dst != nullptr);
    assert(src != nullptr);
    assert(CharBuff_IsValid(*dst));

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

bool CharBuff_StartsWith(const CharBuff s, const CharBuff prefix) {
    if (prefix.len > s.len) {
        return false;
    }

    return memcmp(s.arr, prefix.arr, prefix.len) == 0;
}

size_t CharBuff_WriteLineFromFile(CharBuff* dst, FILE* src, const char separator) {
    assert(dst != nullptr);
    assert(src != nullptr);
    assert(CharBuff_IsValid(*dst));

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

size_t CharBuff_WriteFile(CharBuff* dst, FILE* src) {
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

bool CharBuff_IsValid(const CharBuff s) {
    if (s.arr == nullptr && s.len == 0 && s.cap == 0) {
        return true;
    }
    return s.arr != nullptr && s.len <= s.cap;
}

size_t File_WriteCharBuff(FILE* dst, const CharBuff src) {
    assert(dst != nullptr);
    return fwrite(src.arr, sizeof(char), src.len, dst);
}

size_t CharBuff_WriteF(CharBuff* dst, const char* format, ...) {
    assert(dst != nullptr);
    assert(format != nullptr);
    assert(CharBuff_IsValid(*dst));

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

size_t Str_NoDiffLen(const Str s1, const Str s2) {
    size_t i = 0;
    while (i < s1.len && i < s2.len && Str_At(s1, i) == Str_At(s2, i)) {
        i++;
    }

    return i;
}

void CharBuff_WriteDiff(CharBuff* dst, const Str s1, const Str s2) {
    const auto dstRemCap = dst->cap - dst->len;
    if (dstRemCap < 5 + 1 + 2 + 1 + 2 + 1 + 5) {
        return;
    }

    const size_t contextHalfLen = (dstRemCap - (1 + 2 + 1 + 2 + 1)) / 2;
    const auto   noDiffLen      = Str_NoDiffLen(s1, s2);
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

    const auto srcView = Str_View(s1, srcViewOffset, srcViewOffset + srcViewLen);
    CharBuff_WriteStr(dst, srcView);

    if (srcViewLen - 3 > noDiffLen) {
        CharBuff_WriteStrAt(dst, 0, STR("..."));
    }

    if (noDiffLen == srcMinLen) {
        CharBuff_WriteChar(dst, '{');

        if (s1.len < s2.len) {
            size_t diffViewLen = s2.len - s1.len;
            if (diffViewLen > contextHalfLen) {
                diffViewLen = contextHalfLen;
            }

            CharBuff_WriteStr(dst, STR("\\0|"));
            const auto diffView = Str_View(s2, s1.len, s1.len + diffViewLen);
            CharBuff_WriteStr(dst, diffView);
            CharBuff_WriteChar(dst, '}');
        } else {
            size_t diffViewLen = s1.len - s2.len;
            if (diffViewLen > contextHalfLen) {
                diffViewLen = contextHalfLen;
            }

            const auto diffView = Str_View(s1, s2.len, s2.len + diffViewLen);
            CharBuff_WriteStr(dst, diffView);
            CharBuff_WriteStr(dst, STR("|\\0}"));
        }
        return;
    }

    const auto s1DiffView    = Str_View(s1, noDiffLen, s1.len);
    const auto s2DiffView    = Str_View(s2, noDiffLen, s2.len);
    const auto s1ContextLen  = MinSizeT(contextHalfLen / 2, s1.len - noDiffLen);
    const auto s2ContextLen  = MinSizeT(contextHalfLen / 2, s2.len - noDiffLen);
    const auto s1ContextView = Str_View(s1DiffView, 0, s1ContextLen);
    const auto s2ContextView = Str_View(s2DiffView, 0, s2ContextLen);

    CharBuff_WriteChar(dst, '{');
    CharBuff_WriteStr(dst, s1ContextView);

    if (s1DiffView.len - 3 > s1ContextView.len) {
        CharBuff_WriteStrAt(dst, dst->len - 3, STR("..."));
    }

    CharBuff_WriteChar(dst, '|');
    CharBuff_WriteStr(dst, s2ContextView);
    if (s2DiffView.len - 3 > s2ContextView.len) {
        CharBuff_WriteStrAt(dst, dst->len - 3, STR("..."));
    }
    CharBuff_WriteChar(dst, '}');
}

int CharBuff_Cmp(const CharBuff a, const CharBuff b) {
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
