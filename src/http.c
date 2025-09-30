#include "http.h"

#include <assert.h>

ReqParseResult Req_Parse(Req* dst, Str src) {
    auto endIx = Str_IndexOf(src, STR("\r\n"));
    if (endIx <= 0) {
        return (ReqParseResult){.err = REQ_PARSE_RESULT_BAD_PREAMBLE};
    }

    auto       preambleParts = Strings_OnStack(0, 32);
    const auto preambleStr   = Str_View(src, 0, endIx);
    Strings_Split(&preambleParts, preambleStr, STR(" "));
    if (preambleParts.len != 3) {
        return (ReqParseResult){.err = REQ_PARSE_RESULT_BAD_PREAMBLE};
    }

    dst->method  = Str_Trim(Strings_At(preambleParts, 0), STR(" "));
    dst->path    = Str_Trim(Strings_At(preambleParts, 1), STR(""));
    dst->version = Str_Trim(Strings_At(preambleParts, 2), STR(""));

    while (true) {
        src   = Str_View(src, endIx + 2, src.len);
        endIx = Str_IndexOf(src, STR("\r\n"));
        if (endIx == 0) {
            src = Str_View(src, 2, src.len);
            break;
        }

        auto       headerParts = Strings_OnStack(0, 2);
        const auto headerStr   = Str_View(src, 0, endIx);
        Strings_Split(&headerParts, headerStr, STR(":"));
        if (headerParts.len != 2) {
            return (ReqParseResult){.err = REQ_PARSE_RESULT_BAD_HEADER};
        }

        const Header header = {
            .name  = Str_Trim(Strings_At(headerParts, 0), STR(" ")),
            .value = Str_Trim(Strings_At(headerParts, 1), STR(" ")),
        };

        Headers_Append(&dst->headers, header);
    }

    dst->body = src;
    return (ReqParseResult){.err = REQ_PARSE_RESULT_OK};
}

Header Headers_At(const Headers hs, const size_t i) {
    assert(i < hs.len);
    return hs.arr[i];
}
bool Headers_Append(Headers* dst, Header header) {
    assert(dst != nullptr);
    if (dst->len >= dst->cap) {
        return false;
    }
    dst->arr[dst->len] = header;
    dst->len++;
    return true;
}

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
