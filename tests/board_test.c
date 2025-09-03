#include "board.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "board_json.h"
#include "board_repr.h"

void setUp(void) {}
void tearDown(void) {}

Board parseBoard(const CharSlice src) {
    Board      b   = {};
    const auto res = Squares_Parse(b.squares, src);
    assert(res.err == SQUARES_PARSE_ERR_OK);
    b.state = BOARD_STATE_IN_PROGRESS;
    b.side  = SIDE_WHITE;
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

        SquaresParseResult want;
        Board              wantBoard;
    } test;

    const test tests[] = {
        {
            .name      = "case 0.1",
            .str       = CHAR_SLICE("."),
            .want      = {.err = SQUARES_PARSE_ERR_TOO_SHORT, .offset = 1},
            .wantBoard = {},
        },
        {
            .name      = "case 0.2",
            .str       = CHAR_SLICE(".k"),
            .want      = {.err = SQUARES_PARSE_ERR_TOO_SHORT, .offset = 2},
            .wantBoard = {.squares = {[0][1] = {.side = SIDE_BLACK, .type = PIECE_TYPE_KING}}},
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
            .want      = {.err = SQUARES_PARSE_ERR_UNEXPECTED_CHAR, .offset = 22, .unexpectedChar = 'i'},
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
            .want      = {.err = SQUARES_PARSE_ERR_OK, .offset = 64},
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
            .want      = {.err = SQUARES_PARSE_ERR_OK, .offset = 64 + 7},
            .wantBoard = {},
        },

    };

    bool             failed    = false;
    constexpr size_t testsSize = sizeof(tests) / sizeof(test);
    for (size_t i = 0; i < testsSize; i++) {
        const test tt = tests[i];

        Board      gotBoard = {};
        const auto got      = Squares_Parse(gotBoard.squares, tt.str);

        if (!SquaresParseResult_Equals(got, tt.want)) {
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
        Board       wantB;
    } test;

    const test tests[] =
        {
            {
                .name = "case 1.1: rook, a1a2",
                .m    = parseMove(CHAR_SLICE("a1a2")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[7][0] = WHITE_ROOK, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[6][0] = WHITE_ROOK, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 1.2: rook, a1a8",
                .m    = parseMove(CHAR_SLICE("a1a8")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[7][0] = WHITE_ROOK, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[0][0] = WHITE_ROOK, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 1.3: rook, no obstacle a1h1",
                .m    = parseMove(CHAR_SLICE("a1h1")),
                .b =
                    {.state   = BOARD_STATE_IN_PROGRESS,
                     .side    = SIDE_WHITE,
                     .squares = {[7][0] = WHITE_ROOK, [0][7] = WHITE_KING, [0][0] = BLACK_KING}},
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {.state   = BOARD_STATE_IN_PROGRESS,
                     .side    = SIDE_BLACK,
                     .squares = {[7][7] = WHITE_ROOK, [0][7] = WHITE_KING, [0][0] = BLACK_KING}},
            },
            {
                .name = "case 1.4: rook a1g1 obstacle",
                .m    = parseMove(CHAR_SLICE("a1g1")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[7][0] = WHITE_ROOK, [7][1] = WHITE_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OBSTACLE, .obstacleAt = parsePos(CHAR_SLICE("b1"))},
            },
            {
                .name = "case 1.5: rook, take piece",
                .m    = parseMove(CHAR_SLICE("a2e2")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[6][0] = WHITE_ROOK, [6][4] = BLACK_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK, .pieceTaken = BLACK_PAWN},
                .wantB =
                    {
                    .state   = BOARD_STATE_IN_PROGRESS,
                        .side        = SIDE_BLACK,
                        .squares     = {[6][4] = WHITE_ROOK, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                        .white.taken = {.arr = {PIECE_TYPE_PAWN}, .len = 1},
                    },
            },
            {.name = "case 2.1: bishop, no obstacle",
             .m    = parseMove(CHAR_SLICE("a1b2")),
             .b =
                 {
                     .state   = BOARD_STATE_IN_PROGRESS,
                     .side    = SIDE_WHITE,
                     .squares = {[7][0] = WHITE_BISHOP, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                 },
             .want = {.err = MOVE_ERR_OK},
             .wantB =
                 {
                     .state   = BOARD_STATE_IN_PROGRESS,
                     .side    = SIDE_BLACK,
                     .squares = {[6][1] = WHITE_BISHOP, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                 }},
            {
                .name = "case 2.2: bishop no obstacle",
                .m    = parseMove(CHAR_SLICE("a1h8")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[7][0] = WHITE_BISHOP, [0][0] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[0][7] = WHITE_BISHOP, [0][0] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 2.3: bishop, obstacle",
                .m    = parseMove(CHAR_SLICE("a1c3")),
                .b =
                    {
                        .state = BOARD_STATE_IN_PROGRESS,
                        .side  = SIDE_WHITE,
                        .squares =
                            {[7][0] = WHITE_BISHOP, [6][1] = WHITE_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OBSTACLE, .obstacleAt = parsePos(CHAR_SLICE("b2"))},
            },
            {
                .name = "case 2.4: bishop, take piece",
                .m    = parseMove(CHAR_SLICE("a1c3")),
                .b =
                    {
                        .state = BOARD_STATE_IN_PROGRESS,
                        .side  = SIDE_WHITE,
                        .squares =
                            {[7][0] = WHITE_BISHOP, [5][2] = BLACK_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK, .pieceTaken = {.side = SIDE_BLACK, .type = PIECE_TYPE_PAWN}},
                .wantB =
                    {
                        .state       = BOARD_STATE_IN_PROGRESS,
                        .side        = SIDE_BLACK,
                        .squares     = {[5][2] = WHITE_BISHOP, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                        .white.taken = {.arr = {PIECE_TYPE_PAWN}, .len = 1},
                    },
            },
            {
                .name = "case 4.1 queen, no obstacle",
                .m    = parseMove(CHAR_SLICE("a2a3")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[6][0] = WHITE_QUEEN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[5][0] = WHITE_QUEEN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 4.2 queen, no obstacle",
                .m    = parseMove(CHAR_SLICE("a2b3")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[6][0] = WHITE_QUEEN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[5][1] = WHITE_QUEEN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 4.3 queen, no obstacle",
                .m    = parseMove(CHAR_SLICE("a2g8")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[6][0] = WHITE_QUEEN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[0][6] = WHITE_QUEEN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 4.4 queen, obstacle",
                .m    = parseMove(CHAR_SLICE("a1c3")),
                .b =
                    {
                        .state = BOARD_STATE_IN_PROGRESS,
                        .side  = SIDE_WHITE,
                        .squares =
                            {[7][0] = WHITE_QUEEN, [6][1] = WHITE_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OBSTACLE, .obstacleAt = parsePos(CHAR_SLICE("b2"))},
            },
            {
                .name = "case 4.5 queen, take piece",
                .m    = parseMove(CHAR_SLICE("a1e5")),
                .b =
                    {
                    .state       = BOARD_STATE_IN_PROGRESS,
                        .side = SIDE_WHITE,
                        .squares =
                            {[7][0] = WHITE_QUEEN, [3][4] = BLACK_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK, .pieceTaken = {.side = SIDE_BLACK, .type = PIECE_TYPE_PAWN}},
                .wantB =
                    {
                        .state       = BOARD_STATE_IN_PROGRESS,
                        .side        = SIDE_BLACK,
                        .squares     = {[3][4] = WHITE_QUEEN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                        .white.taken = {.arr = {PIECE_TYPE_PAWN}, .len = 1},
                    },
            },
            {
                .name = "case 5.1: knight, no obstacle",
                .m    = parseMove(CHAR_SLICE("a1b3")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[7][0] = WHITE_KNIGHT, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[5][1] = WHITE_KNIGHT, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 5.3: knight, no obstacle",
                .m    = parseMove(CHAR_SLICE("a1c2")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[7][0] = WHITE_KNIGHT, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[6][2] = WHITE_KNIGHT, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 5.4: knight, take piece",
                .m    = parseMove(CHAR_SLICE("a1b3")),
                .b =
                    {
                        .state = BOARD_STATE_IN_PROGRESS,
                        .side  = SIDE_WHITE,
                        .squares =
                            {[7][0] = WHITE_KNIGHT, [5][1] = BLACK_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK, .pieceTaken = {.side = SIDE_BLACK, .type = PIECE_TYPE_PAWN}},
                .wantB =
                    {
                        .state       = BOARD_STATE_IN_PROGRESS,
                        .side        = SIDE_BLACK,
                        .squares     = {[5][1] = WHITE_KNIGHT, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                        .white.taken = {.arr = {PIECE_TYPE_PAWN}, .len = 1},
                    },
            },
            {
                .name = "case 6.1 pawn, no obstacle",
                .m    = parseMove(CHAR_SLICE("a2a3")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[6][0] = WHITE_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_OK},
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[5][0] = WHITE_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 6.3 pawn, no obstacle",
                .m    = parseMove(CHAR_SLICE("e2e4")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[6][4] = WHITE_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want =
                    {
                        .err = MOVE_ERR_OK,
                    },
                .wantB =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_BLACK,
                        .squares = {[4][4] = WHITE_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
            },
            {
                .name = "case 6.4 pawn, no obstacle, illegal move",
                .m    = parseMove(CHAR_SLICE("e2a5")),
                .b =
                    {
                        .state   = BOARD_STATE_IN_PROGRESS,
                        .side    = SIDE_WHITE,
                        .squares = {[6][4] = WHITE_PAWN, [0][7] = WHITE_KING, [7][7] = BLACK_KING},
                    },
                .want = {.err = MOVE_ERR_ILLEGAL},
            },
        };

    constexpr size_t testsSize = sizeof(tests) / sizeof(test);
    for (size_t i = 0; i < testsSize; i++) {
        const test tt = tests[i];
        // if (strncmp("case 2.4: bishop, take piece", tt.name, 64) != 0) {
        //     continue;
        // }

        TEST_MESSAGE(tt.name);
        Board      gotBoard = tt.b;
        const auto got      = Board_MakeMove(&gotBoard, tt.m);
        TEST_ASSERT_TRUE_MESSAGE(MoveResult_Equals(&tt.want, &got), "incorrect move result");

        if (tt.want.err != MOVE_ERR_OK) {
            TEST_ASSERT_TRUE_MESSAGE(Board_Equals(&tt.b, &gotBoard), "board should not change");
            continue;
        }

        TEST_ASSERT_TRUE_MESSAGE(Board_Equals(&tt.wantB, &gotBoard), "incorrect result board");
    }
}

void Test_WriteAsJson() {
    auto src = parseBoard(CHAR_SLICE(
        "...k...."
        "........"
        "........"
        "........"
        "........"
        "........"
        "....P..."
        "........"
    ));

    src.side        = SIDE_BLACK;
    src.white.taken = (PieceTypes){.arr = {[0] = PIECE_TYPE_KNIGHT}, .len = 1};

    const auto expected = CHAR_SLICE(
        "{"
        "\"next_move_side\":\"BLACK\","
        "\"squares\":{"
        "\"d8\":{\"type\":\"KING\",\"side\":\"BLACK\"},"
        "\"e2\":{\"type\":\"PAWN\",\"side\":\"WHITE\"}"
        "},"
        "\"black\":{\"king_castled\":false,\"taken\":[]},"
        "\"white\":{\"king_castled\":false,\"taken\":[\"KNIGHT\"]}"
        "}"
    );
    auto       dst     = CharSlice_Make(0, 1024);
    auto       js      = JsonStack_Make(0, 128);
    const auto written = CharSlice_WriteBoardAsJson(&dst, &js, &src);

    TEST_ASSERT_GREATER_THAN(0, written);
    TEST_ASSERT_TRUE(CharSlice_Equals(expected, dst));
}

void Test_InterpretJson() {
    const auto src = CHAR_SLICE(
        "{"
        "\"next_move_side\":\"BLACK\","
        "\"squares\":{\"e2\":{\"type\":\"PAWN\",\"side\":\"WHITE\"}},"
        "\"black\":{\"king_castled\":false,\"taken\":[]},"
        "\"white\":{\"king_castled\":false,\"taken\":[\"KNIGHT\"]}"
        "}"
    );
    Board      dst             = {};
    JsonNodes  nodes           = JsonNodes_Make(0, 128);
    const auto jsonParseResult = JsonNodes_Parse(&nodes, src);
    TEST_ASSERT_EQUAL(JSON_PARSE_ERROR_OK, jsonParseResult.err);

    JsonSource jsonSrc = {
        .chars = src,
        .nodes = nodes,
    };

    const auto boardParseResult = Board_InterpretJson(&dst, &jsonSrc);
    TEST_ASSERT_TRUE(boardParseResult);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(Test_Pos_Parse);
    RUN_TEST(Test_Move_Parse);
    RUN_TEST(Test_Board_Parse);
    RUN_TEST(Test_Board_MakeMove);
    RUN_TEST(Test_WriteAsJson);
    RUN_TEST(Test_InterpretJson);
    return UNITY_END();
}
