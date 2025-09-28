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
    REQUEST_PARSE_RESULT_OK = 0,
} ReqParseErr;

typedef struct {
    ReqParseErr err;
    size_t      offset;
} ReqParseResult;

ReqParseResult Req_Parse(Req* dst, Str src);
size_t         CharBuff_WriteHttpStatus(CharBuff* dst, uint32_t status, Str reason, Str version);
size_t         CharBuff_WriteHttpHeader(CharBuff* dst, Header header);
size_t         CharBuff_WriteHttpBody(CharBuff* dst, Str body);

#endif  // HTTP_H
