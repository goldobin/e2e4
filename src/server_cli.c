#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "chars.h"
#include "http.h"

typedef struct {
    size_t id;
    int    socketID;
} ClientContext;

void* handleClient(void* arg) {
    if (arg == nullptr) {
        return nullptr;
    }

    const auto ctx = *(ClientContext*)arg;
    free(arg);

    printf("client %zu connected...\n", ctx.id);

    auto rBuff = CharBuff_OnStack(0, 1024);
    rBuff.len  = recv(ctx.socketID, rBuff.arr, rBuff.cap, 0);

    if (rBuff.len < 0) {
        goto close;
    }

    Req req = {
        .headers = {
            .arr = (Header[10]){},
            .cap = 10,
        }
    };

    const auto res = Req_Parse(&req, CharBuff_View(rBuff, 0, rBuff.len));
    if (res.err != REQ_PARSE_RESULT_OK) {
        printf("client send bad request\n");
        goto close;
    }

    printf("client %zu request:\n%*.s\n", ctx.id, (int)rBuff.len, rBuff.arr);

    auto clientIDBuff = CharBuff_OnStack(0, 32);
    auto wBuff        = CharBuff_OnStack(0, 1024);

    CharBuff_WriteF(&clientIDBuff, "%zu", ctx.id);
    const auto clientIDStr = CharBuff_View(clientIDBuff, 0, clientIDBuff.len);

    CharBuff_WriteHttpStatus(&wBuff, 200, STR("OK"), req.version);
    CharBuff_WriteHttpHeader(&wBuff, (Header){.name = STR("Client-ID"), .value = clientIDStr});
    CharBuff_WriteHttpHeader(&wBuff, (Header){.name = STR("Content-Type"), .value = STR("text/plain")});
    CharBuff_WriteHttpBody(&wBuff, STR("OK"));
    send(ctx.socketID, wBuff.arr, wBuff.len, 0);

close:
    if (close(ctx.socketID) < 0) {
        perror("close");
    }

    printf("client %zu disconnected.\n", ctx.id);
    return nullptr;
}

int main(void) {
    const int                serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    constexpr int            reuseAddr    = 1;
    const struct sockaddr_in address      = {
             .sin_family      = AF_INET,
             .sin_port        = htons(8080),
             .sin_addr.s_addr = INADDR_ANY,
    };
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0) {
        perror("setsockopt");
        return EXIT_FAILURE;
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

        ClientContext* ctx  = malloc(sizeof(ClientContext));
        ctx->id             = clientID;
        ctx->socketID       = clientSocketID;
        pthread_t  threadID = {};
        const auto err      = pthread_create(&threadID, nullptr, handleClient, ctx);
        if (err != 0) {
            fprintf(stderr, "error creating thread: %s\n", strerror(err));
            free(ctx);
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
