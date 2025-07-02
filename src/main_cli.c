#include <assert.h>
#include <stdio.h>

#include "../include/e2/board.h"

#define ANSI_COLOR_RESET       "\x1b[0m"
#define ANSI_COLOR_RED_LOW     "\x1b[0;31m"
#define ANSI_COLOR_GREEN_LOW   "\x1b[0;32m"
#define ANSI_COLOR_YELLOW_LOW  "\x1b[0;33m"
#define ANSI_COLOR_BLUE_LOW    "\x1b[0;34m"
#define ANSI_COLOR_MAGENTA_LOW "\x1b[0;35m"
#define ANSI_COLOR_CYAN_LOW    "\x1b[0;36m"
#define ANSI_COLOR_WHITE_LOW   "\x1b[0;37m"

#define ANSI_COLOR_RED_HIGH     "\x1b[0;91m"
#define ANSI_COLOR_GREEN_HIGH   "\x1b[0;92m"
#define ANSI_COLOR_YELLOW_HIGH  "\x1b[0;93m"
#define ANSI_COLOR_BLUE_HIGH    "\x1b[0;94m"
#define ANSI_COLOR_MAGENTA_HIGH "\x1b[0;95m"
#define ANSI_COLOR_CYAN_HIGH    "\x1b[0;96m"
#define ANSI_COLOR_WHITE_HIGH   "\x1b[0;97m"

void E2_Board_Print(const E2_Board* b) {
    assert(b != nullptr);
    printf(ANSI_COLOR_YELLOW_LOW "  a b c d e f g h " ANSI_COLOR_RESET "\n");

    for (size_t i = 0; i < E2_SIDE_LEN; ++i) {
        for (size_t j = 0; j < E2_SIDE_LEN; ++j) {
            const auto   reverseI = E2_SIDE_LEN - i - 1;
            const E2_Pos pos      = {.row = reverseI, .col = j};
            const auto   piece    = E2_Board_At(b, pos);
            const auto   pieceCh  = E2_PieceType_ToUpperCaseChar(piece.type);
            auto         color    = ANSI_COLOR_WHITE_HIGH;

            if (piece.type == E2_PIECE_TYPE_NONE && (i + j) % 2 == 0) {
                color = ANSI_COLOR_WHITE_LOW;
            } else if (piece.side == E2_SIDE_BLACK) {
                color = ANSI_COLOR_WHITE_LOW;
            }

            if (j == 0) {
                printf(ANSI_COLOR_YELLOW_LOW "%zd " ANSI_COLOR_RESET, reverseI + 1);
            }
            printf("%s%c" ANSI_COLOR_RESET " ", color, pieceCh);
        }
        printf("\n");
    }
}

int main() {
    E2_Move    m1              = {};
    const auto moveParseResult = E2_Move_Parse(&m1, "e2e4");
    char       buff[32]        = {};

    if (moveParseResult.err != E2_MOVE_PARSE_OK) {
        puts("Error parsing move");
        return 1;
    }

    E2_Move_ToString(buff, 32, m1);
    puts(buff);

    E2_Board b = {};
    E2_Board_PlacePieces(&b);
    E2_Board_Print(&b);
}
