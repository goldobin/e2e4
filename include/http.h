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
    uint64_t id;
    Str      method;
    Str      path;
    Str      version;
    Headers  headers;
    Str      body;
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

typedef void (*HandleFn)(void* self, CharBuff* w, const Req* r);

typedef struct {
    uint32_t address;
    uint16_t port;
    void*    handlerData;
    HandleFn handler;
} Server;

typedef struct {
    uint64_t id;
    int      socketID;
    void*    handlerData;
    HandleFn handler;
} ClientData;

Header         Headers_At(Headers hs, size_t i);
bool           Headers_Append(Headers* dst, Header header);
ReqParseResult Header_Parse(Header* dst, Str src);
ReqParseResult Req_Parse(Req* dst, Str src);
size_t         CharBuff_WriteHttpStatus(CharBuff* dst, uint32_t status, Str reason, Str version);
size_t         CharBuff_WriteHttpHeader(CharBuff* dst, Header header);
size_t         CharBuff_WriteHttpBody(CharBuff* dst, Str body);
bool           Server_Start(Server* s);

#endif  // HTTP_H
