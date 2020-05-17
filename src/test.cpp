#include <cstdio>

#include "util/color.h"

#include "test.h"

TestSuite _global_tests = {};

int reg_test(const char *name, TestCallback func, const char *file, unsigned int line) {
    Test t = {};
    t.name = name;
    t.func = func;
    t.file = file;
    t.line = line;
    _global_tests.add(t);
    return 0;
}


void TestSuite::add(Test test) {
    if (tests == nullptr || cap_tests == num_tests) {
        if (cap_tests == 0) { cap_tests = 2; }
        cap_tests *= 2;
        Test *old_tests = tests;

        tests = new Test[cap_tests];
        for (unsigned int i = 0; i < num_tests; i++) {
            tests[i] = old_tests[i];
        }

        if (old_tests) delete[] old_tests;
    }

    test.id = num_tests;
    tests[num_tests++] = test;
}

bool TestSuite::run() {
    std::printf("Running %d tests\n", num_tests);
    unsigned int succeeded = 0;

    GameState state;
    for (unsigned int i = 0; i < num_tests; i++) {
        std::printf(CLEAR "\r%d/%d:  " YELLOW "testing" RESET " %s\r",
                i+1, num_tests, tests[i].name);
        if (tests[i].func(&state)) {
            succeeded++;
        } else {
            std::printf(CLEAR "\r" RED "f" RESET " %s (%s @ %d)\n",
                    tests[i].name, tests[i].file, tests[i].line);
        }
    }
    std::printf(CLEAR);
    std::printf(GREEN "Passed:" RESET " %d\n", succeeded);
    if (succeeded != num_tests)
        std::printf(RED "Failed:" RESET " %d\n", num_tests - succeeded);

    delete[] tests;

    return succeeded == num_tests;
}

