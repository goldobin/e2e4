#include "http.h"

size_t CharBuff_WriteHttpStatus(CharBuff* dst, uint32_t status, Str reason, Str version) {
    return CharBuff_WriteF(
        dst, "%.*s %zu %.*s\r\n", (int)version.len, version.arr, status, (int)reason.len, reason.arr
    );
}
size_t CharBuff_WriteHttpHeader(CharBuff* dst, Header header) {
    return CharBuff_WriteF(
        dst, "%.*s: %.*s\r\n", (int)header.name.len, header.name.arr, (int)header.value.len, header.value.arr
    );
}
size_t CharBuff_WriteHttpBody(CharBuff* dst, const Str body) {
    auto lenBuff = CharBuff_OnStack(0, 32);
    CharBuff_WriteF(&lenBuff, "%zu", body.len);
    auto lenStr = CharBuff_View(lenBuff, 0, lenBuff.len);

    size_t written = 0;
    written += CharBuff_WriteHttpHeader(dst, (Header){.name = STR("Content-Length"), .value = lenStr});
    written += CharBuff_WriteStr(dst, STR("\r\n"));
    written += CharBuff_WriteStr(dst, body);
    return written;
}
