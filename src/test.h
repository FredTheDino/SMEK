#pragma once

#ifndef TESTS
#define TEST_CASE(name, block)
#define TEST_STMT(name, stmt)
#define LOG_TESTS(msg, ...)
#else

#include <cstdio>
#include <vector>
#include <functional>

#include "game.h"

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(a ## b)
#define PP_CAT_II(res) res

#define UNIQUE_NAME(base) PP_CAT(PP_CAT(base, __LINE__), __COUNTER__)

#define TEST_CASE(name, block) static int UNIQUE_NAME(_test_id_) = reg_test((name), [](GameState *game, FILE *report) -> bool block, __FILE__, __LINE__)
#define TEST_STMT(name, stmt) static int UNIQUE_NAME(_test_id_) = reg_test((name), [](GameState *game, FILE *report) -> bool { return stmt; }, __FILE__, __LINE__)
typedef bool(*TestCallback)(GameState *game, FILE *report);

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
    unsigned int run();

    unsigned int cap_tests;
    unsigned int num_tests;
    Test *tests;
};

extern TestSuite _global_tests;

#endif  // ifdef TESTS
