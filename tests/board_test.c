#include "board.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

Board parseBoard(const CharSlice src) {
    Board      b   = {};
    const auto res = Board_Parse(&b, src);

    assert(res.err == BOARD_PARSE_ERR_OK);
    return b;
}

Move parseMove(const CharSlice src) {
    Move       m   = {};
    const auto res = Move_Parse(&m, src);

    assert(res.err == MOVE_PARSE_ERR_OK);
    return m;
}

Pos parsePos(const CharSlice src) {
    Pos        p   = {};
    const auto res = Pos_Parse(&p, src);

    assert(res.err == POS_PARSE_ERR_OK);
    return p;
}

void Test_Pos_Parse(void) {
    typedef struct {
        const char* name;
        CharSlice   str;

        PosParseResult want;
        Pos            wantPos;
    } test;
    const test tests[] = {
        {
            .name    = "case 0.1",
            .str     = CHAR_SLICE("a"),
            .want    = {.err = POS_PARSE_ERR_TOO_SHORT, .offset = 0},
            .wantPos = {.col = 0, .row = 0},
        },
        {
            .name    = "case 0.2",
            .str     = CHAR_SLICE("1"),
            .want    = {.err = POS_PARSE_ERR_TOO_SHORT, .offset = 0},
            .wantPos = {.col = 0, .row = 0},
        },
        {
            .name    = "case 0.3",
            .str     = CHAR_SLICE("k1"),
            .want    = {.err = POS_PARSE_ERR_INVALID_FORMAT, .offset = 0},
            .wantPos = {.col = 0, .row = 0},
        },
        {

            .name    = "case 1.1",
            .str     = CHAR_SLICE("a1"),
            .want    = {.err = POS_PARSE_ERR_OK, .offset = 2},
            .wantPos = {.col = 0, .row = 7},
        },
        {

            .name    = "case 1.1.1",
            .str     = CHAR_SLICE("a1a2"),
            .want    = {.err = POS_PARSE_ERR_OK, .offset = 2},
            .wantPos = {.col = 0, .row = 7},
        },
        {

            .name    = "case 1.2",
            .str     = CHAR_SLICE("a8"),
            .want    = {.err = POS_PARSE_ERR_OK, .offset = 2},
            .wantPos = {.col = 0, .row = 0},
        },
        {

            .name    = "case 1.3",
            .str     = CHAR_SLICE("h8"),
            .want    = {.err = POS_PARSE_ERR_OK, .offset = 2},
            .wantPos = {.col = 7, .row = 0},
        },
        {

            .name    = "case 1.4",
            .str     = CHAR_SLICE("h1"),
            .want    = {.err = POS_PARSE_ERR_OK, .offset = 2},
            .wantPos = {.col = 7, .row = 7},
        },
        {

            .name    = "case 1.5",
            .str     = CHAR_SLICE("e2"),
            .want    = {.err = POS_PARSE_ERR_OK, .offset = 2},
            .wantPos = {.col = 4, .row = 6},
        },
    };

    bool             failed    = false;
    constexpr size_t testsSize = sizeof(tests) / sizeof(test);
    for (size_t i = 0; i < testsSize; i++) {
        const test tt = tests[i];

        Pos        gotPos = {};
        const auto got    = Pos_Parse(&gotPos, tt.str);

        if (!PosParseResult_Equals(got, tt.want)) {
            auto wantStr = CharSlice_Make(0, 128);
            auto gotStr  = CharSlice_Make(0, 128);

            CharSlice_WritePosParseResult(&wantStr, tt.want);
            CharSlice_WritePosParseResult(&gotStr, got);

            TEST_PRINTF("%s - incorrect parse result want: %s, got: %s", tt.name, wantStr.arr, gotStr.arr);
            failed = true;
        }

        if (!Pos_Equals(gotPos, tt.wantPos)) {
            auto wantStr = CharSlice_Make(0, 128);
            auto gotStr  = CharSlice_Make(0, 128);

            CharSlice_WritePos(&wantStr, tt.wantPos);
            CharSlice_WritePos(&gotStr, gotPos);

            TEST_PRINTF("%s - incorrect pos want: %s, got: %s", tt.name, wantStr.arr, gotStr.arr);
            failed = true;
        }
    }

    if (failed) {
        TEST_FAIL();
    }
}

void Test_Move_Parse(void) {
    typedef struct {
        const char* name;
        CharSlice   str;

        MoveParseResult want;
        Move            wantMove;
    } test;

    const test tests[] = {
        {
            .name     = "case 0.1",
            .str      = CHAR_SLICE("a"),
            .want     = {.err = MOVE_PARSE_ERR_TOO_SHORT, .offset = 0},
            .wantMove = {},
        },
        {
            .name     = "case 0.2",
            .str      = CHAR_SLICE("a1"),
            .want     = {.err = MOVE_PARSE_ERR_TOO_SHORT, .offset = 0},
            .wantMove = {},
        },
        {
            .name     = "case 0.3",
            .str      = CHAR_SLICE("k1h1"),
            .want     = {.err = MOVE_PARSE_ERR_INVALID_FROM_FORMAT, .offset = 0},
            .wantMove = {},
        },
        {
            .name     = "case 0.4",
            .str      = CHAR_SLICE("b1i1"),
            .want     = {.err = MOVE_PARSE_ERR_INVALID_TO_FORMAT, .offset = 0},
            .wantMove = {},
        },
        {

            .name     = "case 1.1",
            .str      = CHAR_SLICE("e2e4"),
            .want     = {.err = MOVE_PARSE_ERR_OK, .offset = 4},
            .wantMove = {.from = {.col = 4, .row = 6}, .to = {.col = 4, .row = 4}},
        },
        {

            .name     = "case 1.1.1",
            .str      = CHAR_SLICE("e2e4k1k2"),
            .want     = {.err = MOVE_PARSE_ERR_OK, .offset = 4},
            .wantMove = {.from = {.col = 4, .row = 6}, .to = {.col = 4, .row = 4}},
        },
    };

    bool             failed    = false;
    constexpr size_t testsSize = sizeof(tests) / sizeof(test);
    for (size_t i = 0; i < testsSize; i++) {
        const test tt = tests[i];

        Move       gotMove = {};
        const auto got     = Move_Parse(&gotMove, tt.str);

        if (!MoveParseResult_Equals(got, tt.want)) {
            auto wantStr = CharSlice_Make(0, 128);
            auto gotStr  = CharSlice_Make(0, 128);

            CharSlice_WriteMoveParseResult(&wantStr, tt.want);
            CharSlice_WriteMoveParseResult(&gotStr, got);

            TEST_PRINTF("%s - incorrect parse result want: %s, got: %s", tt.name, wantStr.arr, gotStr.arr);
            failed = true;
        }

        if (!Move_Equals(gotMove, tt.wantMove)) {
            auto wantStr = CharSlice_Make(0, 128);
            auto gotStr  = CharSlice_Make(0, 128);

            CharSlice_WriteMove(&wantStr, tt.wantMove);
            CharSlice_WriteMove(&gotStr, gotMove);

            TEST_PRINTF("%s - incorrect pos want: %s, got: %s", tt.name, wantStr.arr, gotStr.arr);
            failed = true;
        }
    }

    if (failed) {
        TEST_FAIL();
    }
}

void Test_Board_Parse(void) {
    typedef struct {
        const char* name;
        CharSlice   str;

        BoardParseResult want;
        Board            wantBoard;
    } test;

    const test tests[] = {
        {
            .name      = "case 0.1",
            .str       = CHAR_SLICE("."),
            .want      = {.err = BOARD_PARSE_ERR_TOO_SHORT, .offset = 1},
            .wantBoard = {},
        },
        {
            .name      = "case 0.2",
            .str       = CHAR_SLICE(".k"),
            .want      = {.err = BOARD_PARSE_ERR_TOO_SHORT, .offset = 2},
            .wantBoard = {.squares = {[0] = {[1] = {.side = SIDE_BLACK, .type = PIECE_TYPE_KING}}}},
        },
        {
            .name = "case 0.2",
            .str  = CHAR_SLICE(
                "........"
                 "........"
                 ".....i.."
                 "........"
                 "........"
                 "........"
                 "........"
                 "........"
            ),
            .want      = {.err = BOARD_PARSE_ERR_UNEXPECTED_CHAR, .offset = 22, .unexpectedChar = 'i'},
            .wantBoard = {},
        },
        {
            .name = "case 1.1 empty",
            .str  = CHAR_SLICE(
                "........"
                 "........"
                 "........"
                 "........"
                 "........"
                 "........"
                 "........"
                 "........"
            ),
            .want      = {.err = BOARD_PARSE_ERR_OK, .offset = 64},
            .wantBoard = {},
        },
        {
            .name = "case 1.2 empty empty with end lines",
            .str  = CHAR_SLICE(
                "........\n"
                 "........\n"
                 "........\n"
                 "........\n"
                 "........\n"
                 "........\n"
                 "........\n"
                 "........\n"
            ),
            .want      = {.err = BOARD_PARSE_ERR_OK, .offset = 64 + 7},
            .wantBoard = {},
        },

    };

    bool             failed    = false;
    constexpr size_t testsSize = sizeof(tests) / sizeof(test);
    for (size_t i = 0; i < testsSize; i++) {
        const test tt = tests[i];

        Board      gotBoard = {};
        const auto got      = Board_Parse(&gotBoard, tt.str);

        if (!BoardParseResult_Equals(got, tt.want)) {
            auto wantStr = CharSlice_Make(0, 128);
            auto gotStr  = CharSlice_Make(0, 128);

            CharSlice_WriteBoardParseResult(&wantStr, tt.want);
            CharSlice_WriteBoardParseResult(&gotStr, got);

            TEST_PRINTF("%s - incorrect parse result want: %s, got: %s", tt.name, wantStr.arr, gotStr.arr);
            failed = true;
        }

        if (!Board_Equals(&gotBoard, &tt.wantBoard)) {
            auto wantStr = CharSlice_Make(0, 128);
            auto gotStr  = CharSlice_Make(0, 128);

            CharSlice_WriteBoard(&wantStr, &tt.wantBoard);
            CharSlice_WriteBoard(&gotStr, &gotBoard);

            TEST_PRINTF("%s - incorrect pos want: %s, got: %s", tt.name, wantStr.arr, gotStr.arr);
            failed = true;
        }
    }

    if (failed) {
        TEST_FAIL();
    }
}

void Test_Board_MakeMove(void) {
    typedef struct {
        const char* name;
        Board       b;
        Move        m;
        MoveResult  want;
        Board       wantBoard;
    } test;

    const test tests[] = {
        {
            .name = "move rook, case 1.1",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "r......."
            )),
            .m    = parseMove(CHAR_SLICE("a1a2")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "r......."
                "........"
            )),
        },
        {
            .name = "move rook, case 1.2",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "r......."
            )),
            .m    = parseMove(CHAR_SLICE("a1a8")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "r......."
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
            )),
        },
        {
            .name = "move rook, case 1.3",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "r......."
            )),
            .m    = parseMove(CHAR_SLICE("a1h1")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                ".......r"
            )),
        },
        {
            .name      = "move rook, case 2.1",
            .b         = parseBoard(CHAR_SLICE(
                "........"
                        "........"
                        "........"
                        "........"
                        "........"
                        "........"
                        "........"
                        "rp......"
            )),
            .m         = parseMove(CHAR_SLICE("a1g1")),
            .want      = {.err = MOVE_ERR_OBSTACLE, .obstacleAt = parsePos(CHAR_SLICE("b1"))},
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "rp......."
            )),
        },
        {
            .name = "move rook, case 3.1 take piece",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "r...P..."
                   "........"
            )),
            .m    = parseMove(CHAR_SLICE("a2e2")),
            .want =
                {
                    .err        = MOVE_ERR_OK,
                    .pieceTaken = {.side = SIDE_WHITE, .type = PIECE_TYPE_PAWN},
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "....r..."
                "........"
            )),
        },
        {
            .name = "move bishop, case 1.1 no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "b......."
            )),
            .m    = parseMove(CHAR_SLICE("a1b2")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                ".b......"
                "........"
            )),
        },
        {
            .name = "move bishop, case 1.2  no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "b......."
            )),
            .m    = parseMove(CHAR_SLICE("a1h8")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                ".......b"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
            )),
        },
        {
            .name      = "move bishop, case 2.1 obstacle",
            .b         = parseBoard(CHAR_SLICE(
                "........"
                        "........"
                        "........"
                        "........"
                        "........"
                        "........"
                        ".p......"
                        "b......."
            )),
            .m         = parseMove(CHAR_SLICE("a1c3")),
            .want      = {.err = MOVE_ERR_OBSTACLE, .obstacleAt = parsePos(CHAR_SLICE("b2"))},
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                ".p......"
                "b......."
            )),
        },
        {
            .name = "move bishop, case 3.1 take piece",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "..P....."
                   "........"
                   "b......."
            )),
            .m    = parseMove(CHAR_SLICE("a1c3")),
            .want =
                {
                    .err        = MOVE_ERR_OK,
                    .pieceTaken = {.side = SIDE_WHITE, .type = PIECE_TYPE_PAWN},
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "..b....."
                "........"
                "........"
            )),
        },
        {
            .name = "move queen, case 1.1 no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "q......."
            )),
            .m    = parseMove(CHAR_SLICE("a1a2")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "q......."
                "........"
            )),
        },
        {
            .name = "move queen, case 1.2  no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "q......."
            )),
            .m    = parseMove(CHAR_SLICE("a1b2")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                ".q......"
                "........"
            )),
        },
        {
            .name = "move queen, case 1.3 no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "q......."
            )),
            .m    = parseMove(CHAR_SLICE("a1h8")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                ".......q"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
            )),
        },
        {
            .name      = "move queen, case 2.1 obstacle",
            .b         = parseBoard(CHAR_SLICE(
                "........"
                        "........"
                        "........"
                        "........"
                        "........"
                        "........"
                        ".p......"
                        "q......."
            )),
            .m         = parseMove(CHAR_SLICE("a1c3")),
            .want      = {.err = MOVE_ERR_OBSTACLE, .obstacleAt = parsePos(CHAR_SLICE("b2"))},
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                ".p......"
                "q......."
            )),
        },
        {
            .name = "move queen, case 3.1 take piece",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "....P..."
                   "........"
                   "........"
                   "........"
                   "q......."
            )),
            .m    = parseMove(CHAR_SLICE("a1e5")),
            .want =
                {
                    .err        = MOVE_ERR_OK,
                    .pieceTaken = {.side = SIDE_WHITE, .type = PIECE_TYPE_PAWN},
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "....q..."
                "........"
                "........"
                "........"
                "........"
            )),
        },
        {
            .name = "move knight, case 1.1 no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "n......."
            )),
            .m    = parseMove(CHAR_SLICE("a1b3")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                ".n......"
                "........"
                "........"
            )),
        },
        {
            .name = "move knight, case 1.2  no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "n......."
            )),
            .m    = parseMove(CHAR_SLICE("a1c2")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "..n....."
                "........"
            )),
        },
        {
            .name = "move knight, case 3.1 take piece",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   ".P......"
                   "........"
                   "n......."
            )),
            .m    = parseMove(CHAR_SLICE("a1b3")),
            .want =
                {
                    .err        = MOVE_ERR_OK,
                    .pieceTaken = {.side = SIDE_WHITE, .type = PIECE_TYPE_PAWN},
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                ".n......"
                "........"
                "........"
            )),
        },
        {
            .name = "move pawn, case 1.1 no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "P......."
                   "........"
            )),
            .m    = parseMove(CHAR_SLICE("a2a3")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "P......."
                "........"
                "........"
            )),
        },
        {
            .name = "move pawn, case 1.2 no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "....P..."
                   "........"
            )),
            .m    = parseMove(CHAR_SLICE("e2e4")),
            .want =
                {
                    .err = MOVE_ERR_OK,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "....P..."
                "........"
                "........"
                "........"
            )),
        },
        {
            .name = "move pawn, case 1.3 no obstacle",
            .b    = parseBoard(CHAR_SLICE(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "....P..."
                   "........"
            )),
            .m    = parseMove(CHAR_SLICE("e2a5")),
            .want =
                {
                    .err = MOVE_ERR_ILLEGAL,
                },
            .wantBoard = parseBoard(CHAR_SLICE(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "....P..."
                "........"
            )),
        },
    };

    bool             failed    = false;
    constexpr size_t testsSize = sizeof(tests) / sizeof(test);
    for (size_t i = 0; i < testsSize; i++) {
        const test tt = tests[i];

        // if (strncmp("move pawn, case 1.2 no obstacle", tt.name, 32) != 0) {
        //     continue;
        // }
        Board      gotBoard = tt.b;
        const auto got      = Board_MakeMove(&gotBoard, tt.m);

        if (!MoveResult_Equals(&tt.want, &got)) {
            auto wantStr = CharSlice_Make(0, 128);
            auto gotStr  = CharSlice_Make(0, 128);
            CharSlice_WriteMoveResult(&wantStr, &tt.want);
            CharSlice_WriteMoveResult(&gotStr, &got);

            TEST_PRINTF("%s - incorrect move result want: %s, got: %s", tt.name, wantStr.arr, gotStr.arr);
            failed = true;
        };
        if (!Board_Equals(&tt.wantBoard, &gotBoard)) {
            auto wantStr = CharSlice_Make(0, 128);
            auto gotStr  = CharSlice_Make(0, 128);
            CharSlice_WriteBoard(&wantStr, &tt.wantBoard);
            CharSlice_WriteBoard(&gotStr, &gotBoard);

            TEST_PRINTF("%s - incorrect board want: %s, got: %s", tt.name, wantStr.arr, gotStr.arr);
            failed = true;
        };
    }

    if (failed) {
        TEST_FAIL();
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(Test_Pos_Parse);
    RUN_TEST(Test_Move_Parse);
    RUN_TEST(Test_Board_Parse);
    RUN_TEST(Test_Board_MakeMove);
    return UNITY_END();
}
