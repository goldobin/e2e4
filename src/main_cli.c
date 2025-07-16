#include <assert.h>
#include <stdio.h>

#include "board.h"

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

void Board_Print(const Board* b) {
    assert(b != nullptr);
    printf(ANSI_COLOR_YELLOW_LOW "  a b c d e f g h " ANSI_COLOR_RESET "\n");

    for (size_t i = 0; i < BOARD_SIDE_LEN; ++i) {
        for (size_t j = 0; j < BOARD_SIDE_LEN; ++j) {
            const Pos   pos       = {.row = i, .col = j};
            const auto  piece     = Board_At(b, pos);
            const char  rowChar   = (char)(ROW_CHAR_MIN + (BOARD_SIDE_LEN - i - 1));
            auto        color     = ANSI_COLOR_WHITE_HIGH;
            const char* pieceChar = Piece_ToUnicodeChar(piece);

            if (piece.type == PIECE_TYPE_NONE && (i + j) % 2 == 0) {
                color = ANSI_COLOR_WHITE_LOW;
            } else if (piece.side == SIDE_BLACK) {
                color = ANSI_COLOR_WHITE_LOW;
            }

            if (j == 0) {
                printf(ANSI_COLOR_YELLOW_LOW "%c " ANSI_COLOR_RESET, rowChar);
            }
            printf("%s%s" ANSI_COLOR_RESET " ", color, pieceChar);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    Board b    = {};
    Side  side = SIDE_WHITE;

    switch (argc) {
        case 0:
        case 1:
            Board_PlacePieces(&b);
            break;
        case 2:
            const auto filePath = argv[1];
            const auto f        = fopen(filePath, "r");

            if (f == nullptr) {
                printf("Failed to open file %s\n", filePath);
                return 1;
            }

            auto       buff = CharSlice_Make(0, 128);
            const auto read = CharSlice_ReadFile(&buff, f);

            if (read < 1) {
                printf("Empty file %s\n", filePath);
                return 1;
            }

            const auto parseResult = Board_Parse(&b, buff);

            if (parseResult.err != BOARD_PARSE_ERR_OK) {
                auto parseResultStr = CharSlice_Make(0, 128);
                CharSlice_WriteBoardParseResult(&parseResultStr, parseResult);

                printf("Failed to parse file %s: %s\n", filePath, parseResultStr.arr);
                return 1;
            }

            break;

        default:
            printf("Usage: %s [file]\n", argv[0]);
            return 1;
    }

    while (true) {
        Board_Print(&b);

        const char* sideStr = side == SIDE_WHITE ? "white" : "black";
        printf("Next turn [%s]: ", sideStr);
        auto moveInStr = CharSlice_Make(0, 16);

        if (CharSlice_ReadLine(&moveInStr, stdin, '\n') < 4) {
            printf("No move entered\n");
            break;
        };

        Move m = {};
        Move_Parse(&m, moveInStr);
        auto moveStr = CharSlice_Make(0, 16);
        CharSlice_WriteMove(&moveStr, m);

        printf("Parsed move: %s\n", moveStr.arr);

        const Piece p = Board_At(&b, m.from);
        if (p.type == PIECE_TYPE_NONE) {
            printf("No piece to move\n");
            continue;
        }

        if (p.side != side) {
            const char* pieceSideStr = p.side == SIDE_WHITE ? "white" : "black";
            printf("Can't move %s piece\n", pieceSideStr);
            continue;
        }

        const auto moveResult    = Board_MakeMove(&b, m);
        auto       moveResultStr = CharSlice_Make(0, 32);
        CharSlice_WriteMoveResult(&moveResultStr, &moveResult);

        if (moveResult.err != MOVE_ERR_OK) {
            printf("Move result: %s\n", moveResultStr.arr);
            continue;
        }

        side = side == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
    }
}
