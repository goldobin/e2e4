#include "../include/http.h"

#include <assert.h>
#include <stdlib.h>
#include <unity.h>

#include "../include/arena.h"
#include "../include/chars.h"

auto mem = Arena_OnStack(1024);

void setUp() {}
void tearDown() {}

void Test_Req_Parse() {
    const auto src =
        STR("GET /some/url HTTP/1.1\r\n"
            "Header-01: value01\r\n"
            "\r\n"
            "some body text");

    Req req = {
        .headers = {
            .arr = (Header[10]){},
            .cap = 10,
        }
    };

    const auto res = Req_Parse(&req, src);
    TEST_ASSERT_EQUAL(REQ_PARSE_RESULT_OK, res.err);
    TEST_ASSERT_TRUE(Str_Equals(STR("GET"), req.method));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(Test_Req_Parse);

    printf("mem: %zd/%zd\n", mem.offset, mem.cap);

    return UNITY_END();
}
