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

)

file(GLOB_RECURSE FORMAT_SOURCES
        src/*.c
        src/e2/*.c
        src/u/*.c
        include/e2/*.h
        include/u/*.h
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
add_executable(e2e4_cli
        src/main_cli.c
)
target_compile_features(e2e4_cli PRIVATE ${C_STANDARD})
target_compile_options(e2e4_cli PRIVATE ${CC_PARAMS})
target_link_libraries(e2e4_cli PRIVATE
        e2e4
)


# e2e4 LIB
add_library(e2e4 STATIC
        src/e2/board.c
        src/u/slice.c
        src/u/arena.c
)
target_compile_features(e2e4 PRIVATE ${C_STANDARD})
target_compile_options(e2e4 PRIVATE ${CC_PARAMS})
add_dependencies(e2e4 format)


# Testing
include(CTest)

add_executable(u_slice_test
        tests/u_slice_test.c
)

target_compile_features(u_slice_test PRIVATE ${C_STANDARD})
target_compile_options(u_slice_test PRIVATE ${CC_PARAMS})
target_link_libraries(u_slice_test PRIVATE
        e2e4
        unity
)

add_executable(u_arena_test
        tests/u_arena_test.c
)

target_compile_features(u_arena_test PRIVATE ${C_STANDARD})
target_compile_options(u_arena_test PRIVATE ${CC_PARAMS})
target_link_libraries(u_arena_test PRIVATE
        e2e4
        unity
)

add_executable(e2e4_board_test
        tests/e2_board_test.c
)

target_compile_features(e2e4_board_test PRIVATE ${C_STANDARD})
target_compile_options(e2e4_board_test PRIVATE ${CC_PARAMS})
target_link_libraries(e2e4_board_test PRIVATE
        e2e4
        unity
)

add_test(NAME "Board" COMMAND e2e4_board_test)

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
