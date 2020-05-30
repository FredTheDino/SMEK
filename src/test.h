#pragma once

#ifndef TESTS
#define TEST_CASE(name, block)
#define TEST_STMT(name, stmt)
#define LOG_TESTS(msg, ...)
#else

///# Tests
// Tests are declared inline with the code in question,
// making it as easy as possible to write them.
//
// Tests are run with the scons-target <code>tests</code>
// (<code>scons tests</code>). There are some flags
// available:
//
// <ul>
//   <li> <code>--ci</code>: Disable the use of
//   <code>\r</code>, making the output more readable for
//   automated systems. </li>
//   <li> <code>--report</code>: Write a verbose report to a text
//   file (<code>bin/tests/report.txt</code>). Tests can
//   write to this report by calling <code>LOG_TESTS</code>
//   (see below).
// </ul>
//
// The tests run automatically for all commits to master and
// pull requests targeting master. If any tests fail the
// built binary, its assets and the report is uploaded as
// artifacts.

#include <cstdio>
#include <vector>
#include <functional>
#include <cstring>

#include "game.h"

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(a ## b)
#define PP_CAT_II(res) res

#define UNIQUE_NAME(base) PP_CAT(PP_CAT(base, __LINE__), __COUNTER__)

#define TEST_CASE(name, block) static int UNIQUE_NAME(_test_id_) = reg_test((name), [](GameState *game, FILE *report) -> bool block, __FILE__, __LINE__)
#define TEST_STMT(name, stmt) static int UNIQUE_NAME(_test_id_) = reg_test((name), [](GameState *game, FILE *report) -> bool { return stmt; }, __FILE__, __LINE__)
typedef bool(*TestCallback)(GameState *game, FILE *report);

#define TEST_FORMAT(IN, ARGS, EXPECTED) TEST_CASE("format " STR(IN), {  \
    char buffer[64] = {};                                               \
    format(buffer, 64, ARGS, IN);                                       \
    const char *expected = EXPECTED;                                    \
    if (std::strcmp(buffer, expected) != 0) {                           \
        LOG_TESTS("got '%s', expected '%s'", buffer, expected);         \
        return false;                                                   \
    }                                                                   \
    return true;                                                        \
})

#define LOG_TESTS(msg, ...) if (report) { std::fprintf(report, msg "\n", ##__VA_ARGS__); }  // matches the normal LOG

int reg_test(const char *name, TestCallback func, const char *file, unsigned int line);

struct Test {
    unsigned int id;
    const char *name;
    TestCallback func;
    const char *file;
    unsigned int line;
};

struct TestSuite {
    void add(Test test);
    unsigned int run(bool ci, bool write_report, const char *path);

    unsigned int cap_tests;
    unsigned int num_tests;
    Test *tests;
};

extern TestSuite _global_tests;

#if 0

///*
// Creates a test that succeedes iff <code>block</code>
// returns true.
TEST_CASE(name, block)

///*
// Creates a test that succeedes iff <code>statement</code>
// is evaluated to true.
TEST_STMT(name, statement)

///*
// Write a string to the report, if it is being written to.
// Can be used for verbose logging in larger tests where it
// might be useful to know the values of variables and such.
LOG_TESTS(msg, ...)

#endif

#endif  // ifdef TESTS
