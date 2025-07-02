#include "u/slice.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "u/func.h"

CharSlice CharSlice_New(char* buffer, const size_t len, const size_t cap) {
    assert(buffer != nullptr);
    assert(len <= cap);

    return (CharSlice){
        .chars = buffer,
        .len   = len,
        .cap   = cap,
    };
}

char CharSlice_At(const CharSlice* s, const size_t i) {
    assert(s != nullptr);
    assert(i < s->len);
    return s->chars[i];
}

size_t CharSlice_WriteOneAt(CharSlice* s, const size_t offset, const char ch) {
    assert(s != nullptr);
    if (offset > s->len || offset >= s->cap) {
        return 0;
    }
    s->chars[offset] = ch;
    if (offset == s->len) {
        s->len++;
    }
    return 1;
}

size_t CharSlice_WriteAt(CharSlice* s, const size_t offset, const CharSlice other) {
    assert(s != nullptr);
    if (offset > s->len || offset >= s->cap) {
        return 0;
    }

    const auto charsToCopy = MinSizeT(s->cap - offset, other.len);
    if (charsToCopy == 0) {
        return 0;
    }

    memcpy(&s->chars[offset], &other.chars[0], charsToCopy);
    s->len = offset + charsToCopy;
    return charsToCopy;
}

size_t CharSlice_WriteOne(CharSlice* s, const char ch) { return CharSlice_WriteOneAt(s, s->len, ch); }

size_t CharSlice_Write(CharSlice* s, const CharSlice other) {
    assert(s != nullptr);
    return CharSlice_WriteAt(s, s->len, other);
}

CharSlice CharSlice_View(const CharSlice* s, const size_t start, const size_t end) {
    assert(s != nullptr);
    assert(start <= end);
    assert(end <= s->len);
    const auto len = end - start;
    return (CharSlice){
        .chars = &s->chars[start],
        .cap   = len,
        .len   = len,
    };
}

size_t CharSlice_ToString(char* dst, const size_t maxLen, CharSlice slice) {
    assert(dst != nullptr);

    const auto dstLen = MinSizeT(slice.len, maxLen - 1);
    if (slice.len >= maxLen) {
        return false;
    }

    memcpy(dst, slice.chars, dstLen);
    dst[dstLen] = '\0';
    return true;
}

size_t CharSlice_NoDiffLen(const CharSlice s1, const CharSlice s2) {
    size_t i = 0;
    while (i < s1.len && i < s2.len && CharSlice_At(&s1, i) == CharSlice_At(&s2, i)) {
        i++;
    }

    return i;
}

void CharSlice_WriteDiff(CharSlice* dst, const CharSlice s1, const CharSlice s2) {
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

    const auto srcView = CharSlice_View(&s1, srcViewOffset, srcViewOffset + srcViewLen);
    CharSlice_Write(dst, srcView);

    if (srcViewLen - 3 > noDiffLen) {
        CharSlice_WriteStringAt(dst, 0, "...");
    }

    if (noDiffLen == srcMinLen) {
        CharSlice_WriteOne(dst, '{');

        if (s1.len < s2.len) {
            size_t diffViewLen = s2.len - s1.len;
            if (diffViewLen > contextHalfLen) {
                diffViewLen = contextHalfLen;
            }

            CharSlice_WriteString(dst, "\\0|");
            const auto diffView = CharSlice_View(&s2, s1.len, s1.len + diffViewLen);
            CharSlice_Write(dst, diffView);
            CharSlice_WriteOne(dst, '}');
        } else {
            size_t diffViewLen = s1.len - s2.len;
            if (diffViewLen > contextHalfLen) {
                diffViewLen = contextHalfLen;
            }

            const auto diffView = CharSlice_View(&s1, s2.len, s2.len + diffViewLen);
            CharSlice_Write(dst, diffView);
            CharSlice_WriteString(dst, "|\\0}");
        }
        return;
    }

    const auto s1DiffView    = CharSlice_View(&s1, noDiffLen, s1.len);
    const auto s2DiffView    = CharSlice_View(&s2, noDiffLen, s2.len);
    const auto s1ContextLen  = MinSizeT(contextHalfLen / 2, s1.len - noDiffLen);
    const auto s2ContextLen  = MinSizeT(contextHalfLen / 2, s2.len - noDiffLen);
    const auto s1ContextView = CharSlice_View(&s1DiffView, 0, s1ContextLen);
    const auto s2ContextView = CharSlice_View(&s2DiffView, 0, s2ContextLen);

    CharSlice_WriteOne(dst, '{');
    CharSlice_Write(dst, s1ContextView);

    if (s1DiffView.len - 3 > s1ContextView.len) {
        CharSlice_WriteStringAt(dst, dst->len - 3, "...");
    }

    CharSlice_WriteOne(dst, '|');
    CharSlice_Write(dst, s2ContextView);
    if (s2DiffView.len - 3 > s2ContextView.len) {
        CharSlice_WriteStringAt(dst, dst->len - 3, "...");
    }
    CharSlice_WriteOne(dst, '}');
}
