#pragma once
#include <vector>
#include <functional>

#include "main.h"

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(a ## b)
#define PP_CAT_II(res) res

#define UNIQUE_NAME(base) PP_CAT(PP_CAT(base, __LINE__), __COUNTER__)

#define _FILE() PP_CAT_II(__FILE__)
#define _LINE() PP_CAT_II(__LINE__)

#define TEST_CASE(name, block) int UNIQUE_NAME(_test_id_) = reg_test((name), [](GameState *game) -> bool block, _FILE(), _LINE());
typedef bool(*TestCallback)(GameState *game);

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
    void run();

    unsigned int cap_tests;
    unsigned int num_tests;
    Test *tests;
};

#ifdef TESTS
extern TestSuite _global_tests;
#else
inline int reg_test(const char *, TestCallback, const char *, unsigned int) { return 0; }
#endif
