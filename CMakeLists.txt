cmake_minimum_required(VERSION 3.30.5)

enable_testing()

project(e2e4
        VERSION 1.0
        DESCRIPTION "E2E4 Game"
        LANGUAGES C)

set(C_STANDARD c_std_23)
set(CC_PARAMS
        -v
        -Wall
        -Wextra
        -Wpedantic
        -Wformat-security
        -Wstrict-prototypes
        -D_XOPEN_SOURCE=700
)

set(CC_TEST_PARAMS
        ${CC_PARAMS}
        -Wno-gnu-zero-variadic-macro-arguments
)

file(GLOB_RECURSE FORMAT_SOURCES
        src/*.c
        include/*.h
        tests/*.c
)


find_program(CLANG_FORMAT_EXECUTABLE
        NAMES clang-format clang-format-20
        DOC "Path to clang-format executable"
)

if (CLANG_FORMAT_EXECUTABLE)
    add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${FORMAT_SOURCES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting source code with clang-format"
            VERBATIM
    )
else ()
    message(FATAL_ERROR "clang-format not found.")
endif ()


include_directories(include)

# Main CLI executable
add_executable(e2e4_cli src/main_cli.c)
target_compile_features(e2e4_cli PRIVATE ${C_STANDARD})
target_compile_options(e2e4_cli PRIVATE ${CC_PARAMS})
target_link_libraries(e2e4_cli PRIVATE e2e4)

# Server test executable
add_executable(e2e4_server src/server_cli.c)
target_compile_features(e2e4_server PRIVATE ${C_STANDARD})
target_compile_options(e2e4_server PRIVATE ${CC_TEST_PARAMS})
target_link_libraries(e2e4_server PRIVATE e2e4)


# e2e4 LIB
add_library(e2e4 STATIC
        src/arena.c
        src/chars.c
        src/board.c
        src/json.c
        src/board_json.c
        src/board_repr.c
        src/http.c
)

target_compile_features(e2e4 PRIVATE ${C_STANDARD})
target_compile_options(e2e4 PRIVATE ${CC_PARAMS})
add_dependencies(e2e4 format)


# Testing
include(CTest)

add_executable(chars_test tests/chars_test.c)
target_compile_features(chars_test PRIVATE ${C_STANDARD})
target_compile_options(chars_test PRIVATE ${CC_TEST_PARAMS})
target_link_libraries(chars_test PRIVATE e2e4 unity)

add_executable(arena_test tests/arena_test.c)
target_compile_features(arena_test PRIVATE ${C_STANDARD})
target_compile_options(arena_test PRIVATE ${CC_TEST_PARAMS})
target_link_libraries(arena_test PRIVATE e2e4 unity)

add_executable(board_test tests/board_test.c)
target_compile_features(board_test PRIVATE ${C_STANDARD})
target_compile_options(board_test PRIVATE ${CC_TEST_PARAMS})
target_link_libraries(board_test PRIVATE e2e4 unity)

add_executable(json_test tests/json_test.c)
target_compile_features(json_test PRIVATE ${C_STANDARD})
target_compile_options(json_test PRIVATE ${CC_TEST_PARAMS})
target_link_libraries(json_test PRIVATE e2e4 unity)

add_test(NAME "Arena" COMMAND arena_test)
add_test(NAME "Chars" COMMAND chars_test)
add_test(NAME "Board" COMMAND board_test)
add_test(NAME "JSON" COMMAND json_test)

# Adding Deps
include(FetchContent)

#FetchContent_Declare(
#        raylib
#        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
#        GIT_TAG "5.5"
#        GIT_PROGRESS TRUE
#)
#FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
        unity
        GIT_REPOSITORY "https://github.com/ThrowTheSwitch/Unity.git"
        GIT_TAG "v2.6.1"
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(unity)
target_compile_definitions(unity PUBLIC UNITY_INCLUDE_PRINT_FORMATTED)

