#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

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

void printBoard(const Board* b) {
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

bool readBoardFromFile(Board* dst, const CharSlice filePath) {
    assert(dst != nullptr);
    assert(CharSlice_IsValid(&filePath));
    assert(CharSlice_IsNullTerminated(&filePath));

    const auto f = fopen(filePath.arr, "r");
    if (f == nullptr) {
        fprintf(stderr, "Failed to open file %s to load the board: %s\n", filePath.arr, strerror(errno));
        return false;
    }

    auto       buff = CharSlice_Make(0, 128);
    const auto read = CharSlice_ReadFile(&buff, f);

    if (fclose(f) != 0) {
        fprintf(stderr, "Failed to close file %s: %s\n", filePath.arr, strerror(errno));
        return false;
    }

    if (read < 1) {
        printf("File is empty %s\n", filePath.arr);
        return false;
    }

    const auto parseResult = Board_Parse(dst, buff);

    if (parseResult.err != BOARD_PARSE_ERR_OK) {
        auto parseResultStr = CharSlice_Make(0, 128);
        CharSlice_WriteBoardParseResult(&parseResultStr, parseResult);

        printf("Failed to parse file %s: %s\n", filePath.arr, parseResultStr.arr);
        return false;
    }

    return true;
}

bool writeBoardToFile(const CharSlice filePath, const Board* board) {
    assert(CharSlice_IsValid(&filePath));
    assert(CharSlice_IsNullTerminated(&filePath));
    assert(board != nullptr);

    auto buff = CharSlice_Make(0, 128);
    CharSlice_WriteBoard(&buff, board);

    const auto f = fopen(filePath.arr, "w");
    if (f == nullptr) {
        fprintf(stderr, "Failed to open file %s save the board: %s\n", filePath.arr, strerror(errno));
        return false;
    }
    const auto written = File_WriteCharSlice(f, buff);
    if (fclose(f) != 0) {
        fprintf(stderr, "Failed to close file %s: %s\n", filePath.arr, strerror(errno));
        return false;
    }

    if (written < buff.len) {
        fprintf(
            stderr, "Failed to fully write board to file %s. Only %zd out of %zd bytes written\n", filePath.arr,
            written, buff.len
        );
        return false;
    }
    return true;
}

void printUsage(const char* progName) { printf("Usage: %s [file]\n", progName); }

int main(const int argc, char* argv[]) {
    Board b    = {};
    Side  side = SIDE_WHITE;

    switch (argc) {
        case 0:
        case 1:
            Board_PlacePieces(&b);
            break;
        case 2:
            auto filePath = CharSlice_Make(0, 256);
            CharSlice_WriteStr(&filePath, argv[1], 256);
            if (!readBoardFromFile(&b, filePath)) {
                return 1;
            }
            break;

        default:
            printUsage(argv[0]);
            return 1;
    }

    while (true) {
        printBoard(&b);

        const char* sideStr = side == SIDE_WHITE ? "white" : "black";
        printf(
            "Next turn for" ANSI_COLOR_YELLOW_HIGH " %s " ANSI_COLOR_RESET "or command (save <path> | quit): ", sideStr
        );

        auto in = CharSlice_Make(0, 128);
        if (CharSlice_ReadLine(&in, stdin, '\n') < 4) {
            printf("No move or command entered\n");
            break;
        };

        if (CharSlice_StartsWith(&in, CHAR_SLICE("save "))) {
            const auto filePath = CharSlice_View(&in, 5, in.len);
            if (!writeBoardToFile(filePath, &b)) {
                printf("Failed to save board to %s\n", filePath.arr);
                continue;
            }

            printf("Board saved to %s\n", filePath.arr);
            continue;
        }

        if (CharSlice_StartsWith(&in, CHAR_SLICE("quit"))) {
            printf("Bye!\n");
            return 0;
        }

        Move       m               = {};
        const auto moveParseResult = Move_Parse(&m, in);
        if (moveParseResult.err != MOVE_PARSE_ERR_OK) {
            auto moveParseResultStr = CharSlice_Make(0, 128);
            CharSlice_WriteMoveParseResult(&moveParseResultStr, moveParseResult);
            printf("Failed to parse move: %s\n", moveParseResultStr.arr);
            continue;
        }

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

    return 0;
}
