#include "test.h"

TestSuite _global_tests = {};

int reg_test(const char *name, TestCallback func) {
    Test t = {};
    t.name = name;
    t.func = func;
    _global_tests.add(t);
    return 0;
}

#include <stdio.h>

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

void TestSuite::run() {
    GameState state;
    printf("Running tests\n");
    unsigned int succeeded = 0;
    for (unsigned int i = 0; i < num_tests; i++) {
        bool success = tests[i].func(&state);
        printf("  %d: %s %d\n", tests[i].id, tests[i].name, success);
        succeeded += success;
    }
    printf("%d/%d\n", succeeded, num_tests);
    delete[] tests;
}

