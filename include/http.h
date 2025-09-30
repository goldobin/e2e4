#ifndef HTTP_H
#define HTTP_H

#include <chars.h>

typedef struct {
    Str name;
    Str value;
} Header;

typedef struct {
    Header* arr;
    size_t  len;
    size_t  cap;
} Headers;

typedef struct {
    Str     method;
    Str     path;
    Str     version;
    Headers headers;
    Str     body;
} Req;

typedef enum {
    REQ_PARSE_RESULT_OK           = 0,
    REQ_PARSE_RESULT_BAD_PREAMBLE = 1,
    REQ_PARSE_RESULT_NO_METHOD    = 2,
    REQ_PARSE_RESULT_BAD_HEADER   = 3,
} ReqParseErr;

typedef struct {
    ReqParseErr err;
    size_t      offset;
} ReqParseResult;

Header         Headers_At(Headers hs, size_t i);
bool           Headers_Append(Headers* dst, Header header);
size_t         CharBuff_WriteHttpStatus(CharBuff* dst, uint32_t status, Str reason, Str version);
ReqParseResult Req_Parse(Req* dst, Str src);
ReqParseResult Header_Parse(Header* dst, Str src);
size_t         CharBuff_WriteHttpHeader(CharBuff* dst, Header header);
size_t         CharBuff_WriteHttpBody(CharBuff* dst, Str body);

#endif  // HTTP_H
