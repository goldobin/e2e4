#include "http.h"

#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

void* handleClient(void* arg) {
    if (arg == nullptr) {
        return nullptr;
    }

    const auto data = *(ClientData*)arg;
    free(arg);

    printf("client %" PRIu64 " connected...\n", data.id);

    auto rBuff = CharBuff_OnStack(0, 1024);
    rBuff.len  = recv(data.socketID, rBuff.arr, rBuff.cap, 0);

    if (rBuff.len < 0) {
        goto close;
    }

    Req req = {
        .id      = data.id,
        .headers = {.arr = (Header[10]){}, .cap = 10},
    };

    const auto res = Req_Parse(&req, CharBuff_View(rBuff, 0, rBuff.len));
    if (res.err != REQ_PARSE_RESULT_OK) {
        printf("client send bad request\n");
        goto close;
    }

    printf("client %" PRIu64 " request:\n%*.s\n", data.id, (int)rBuff.len, rBuff.arr);
    auto wBuff = CharBuff_OnStack(0, 1024);

    data.handler(data.handlerData, &wBuff, &req);
    send(data.socketID, wBuff.arr, wBuff.len, 0);

close:
    if (close(data.socketID) < 0) {
        perror("close");
    }

    printf("client %" PRIu64 " disconnected.\n", data.id);
    return nullptr;
}

bool Server_Start(Server* s) {
    const int                serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    constexpr int            reuseAddr    = 1;
    const struct sockaddr_in address      = {
             .sin_family      = AF_INET,
             .sin_port        = htons(s->port),
             .sin_addr.s_addr = s->address,
    };
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0) {
        perror("setsockopt");
        return false;
    }

    // Bind server_socket to address
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        return EXIT_FAILURE;
    }

    // Listen for clients and allow the accept function to be used
    // Allow 4 clients to be queued while the server processes
    if (listen(serverSocket, 4) == -1) {
        perror("listen");
        return EXIT_FAILURE;
    }

    size_t clientID = 0;
    while (clientID < 1000) {
        clientID += 1;
        // Wait for a client to connect, then open a socket
        const int clientSocketID = accept(serverSocket, nullptr, nullptr);

        if (clientSocketID == -1) {
            perror("accept");
            continue;
        }

        ClientData* data    = malloc(sizeof(ClientData));
        data->id            = clientID;
        data->socketID      = clientSocketID;
        data->handler       = s->handler;
        data->handlerData   = s->handlerData;
        pthread_t  threadID = {};
        const auto err      = pthread_create(&threadID, nullptr, handleClient, data);
        if (err != 0) {
            fprintf(stderr, "error creating thread: %s\n", strerror(err));
            free(data);
            continue;
        }
        pthread_detach(threadID);
    }

    if (close(serverSocket) == -1) {
        perror("close");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
