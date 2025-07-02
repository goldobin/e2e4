#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "e2/board.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

E2_Board parseBoard(const char src[static 1]) {
    E2_Board   b   = {};
    const auto res = E2_Board_Parse(&b, src);

    assert(res.err == E2_BOARD_PARSE_OK);
    return b;
}

E2_Move parseMove(const char src[static 1]) {
    E2_Move    m   = {};
    const auto res = E2_Move_Parse(&m, src);

    assert(res.err == E2_MOVE_PARSE_OK);
    return m;
}

E2_Pos parsePos(const char src[static 1]) {
    E2_Pos     p   = {};
    const auto res = E2_Pos_Parse(&p, src);

    assert(res.err == E2_POS_PARSE_OK);
    return p;
}

void Test_E2_Board_Move(void) {
    typedef struct {
        const char*         name;
        E2_Board            b;
        E2_Move             m;
        E2_Board_MoveResult want;
        E2_Board            wantB;
    } test;

    const test tests[] = {
        {
            .name = "move rook, case 1.1",
            .b    = parseBoard(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "r......."
            ),
            .m = parseMove("a1a2"),
            .want =
                {
                    .err = E2_BOARD_MOVE_OK,
                },
            .wantB = parseBoard(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "r......."
                "........"
            ),
        },
        {
            .name = "move rook, case 1.2",
            .b    = parseBoard(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "r......."
            ),
            .m = parseMove("a1a8"),
            .want =
                {
                    .err = E2_BOARD_MOVE_OK,
                },
            .wantB = parseBoard(
                "r......."
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
            ),
        },
        {
            .name = "move rook, case 1.3",
            .b    = parseBoard(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "r......."
            ),
            .m = parseMove("a1h1"),
            .want =
                {
                    .err = E2_BOARD_MOVE_OK,
                },
            .wantB = parseBoard(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                ".......r"
            ),
        },
        {
            .name = "move rook, case 2.1",
            .b    = parseBoard(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "rp......"
            ),
            .m     = parseMove("a1g1"),
            .want  = {.err = E2_BOARD_MOVE_OBSTACLE, .obstacleAt = parsePos("b1")},
            .wantB = parseBoard(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "rp......."
            ),
        },
        {
            .name = "move bishop, case 1",
            .b    = parseBoard(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "b......."
            ),
            .m = parseMove("a1b2"),
            .want =
                {
                    .err = E2_BOARD_MOVE_OK,
                },
            .wantB = parseBoard(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                ".b......"
                "........"
            ),
        },
        {
            .name = "move queen, case 1.1",
            .b    = parseBoard(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "q......."
            ),
            .m = parseMove("a1a2"),
            .want =
                {
                    .err = E2_BOARD_MOVE_OK,
                },
            .wantB = parseBoard(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "q......."
                "........"
            ),
        },
        {
            .name = "move queen, case 1.2",
            .b    = parseBoard(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "q......."
            ),
            .m = parseMove("a1b2"),
            .want =
                {
                    .err = E2_BOARD_MOVE_OK,
                },
            .wantB = parseBoard(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                ".q......"
                "........"
            ),
        },
        {
            .name = "move knight, case 1.1",
            .b    = parseBoard(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "n......."
            ),
            .m = parseMove("a1b3"),
            .want =
                {
                    .err = E2_BOARD_MOVE_OK,
                },
            .wantB = parseBoard(
                "........"
                "........"
                "........"
                "........"
                "........"
                ".n......"
                "........"
                "........"
            ),
        },
        {
            .name = "move knight, case 1.2",
            .b    = parseBoard(
                "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "........"
                   "n......."
            ),
            .m = parseMove("a1c2"),
            .want =
                {
                    .err = E2_BOARD_MOVE_OK,
                },
            .wantB = parseBoard(
                "........"
                "........"
                "........"
                "........"
                "........"
                "........"
                "..n....."
                "........"
            ),
        },
    };

    constexpr size_t testsSize = sizeof(tests) / sizeof(test);

    for (size_t i = 0; i < testsSize; i++) {
        const test tt = tests[i];

        if (strncmp("move rook, case 2.1", tt.name, 32) != 0) {
            continue;
        }

        TEST_MESSAGE(tt.name);
        E2_Board   gotB = tt.b;
        const auto got  = E2_Board_Move(&gotB, tt.m);

        TEST_ASSERT_TRUE_MESSAGE(E2_Board_MoveResult_Equals(&tt.want, &got), "move result is not what expected");
        TEST_ASSERT_TRUE_MESSAGE(E2_Board_Equals(&tt.wantB, &gotB), "board is not what expected");
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(Test_E2_Board_Move);
    return UNITY_END();
}
