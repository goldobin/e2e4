#include <inttypes.h>

#include "chars.h"
#include "http.h"

typedef struct {
    Str name;
} Service;

void handle(void* self, CharBuff* w, const Req* r) {
    auto s = (Service*)self;

    auto clientIDBuff = CharBuff_OnStack(0, 32);
    CharBuff_WriteF(&clientIDBuff, "%" PRIu64, r->id);

    CharBuff_WriteHttpStatus(w, 200, STR("OK"), r->version);
    CharBuff_WriteHttpHeader(w, (Header){.name = STR("Client-ID"), .value = CharBuff_ToStr(clientIDBuff)});
    CharBuff_WriteHttpHeader(w, (Header){.name = STR("Content-Type"), .value = STR("text/plain")});
    CharBuff_WriteHttpBody(w, s->name);
}

int main(void) {
    Service s   = {.name = STR("E2E4 NPC Server")};
    Server  srv = {
         .port        = 8080,
         .handlerData = &s,
         .handler     = &handle,
    };

    Server_Start(&srv);
}
