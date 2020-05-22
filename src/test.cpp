#include <cstdio>
#include <stdexcept>

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

#define STREAM stderr
#ifdef CI
#define PRE ""
#define POST "\n"
#else
#define PRE CLEAR "\r"
#define POST "\r"
#endif

unsigned int TestSuite::run() {
    //TODO(gu) command line flag
#ifdef REPORT
    FILE *report = std::fopen("report.txt", "w");  // TODO(gu) absolute path
#else
    FILE *report = nullptr;
#endif

    std::fprintf(STREAM, "Running %d tests\n", num_tests);
    unsigned int succeeded = 0;

    for (unsigned int i = 0; i < num_tests; i++) {
        GameState state = {};
        std::fprintf(STREAM, PRE "%d/%d: %s" POST, i+1, num_tests, tests[i].name);
        if (report)
            std::fprintf(report, "%d/%d: %s\n", i+1, num_tests, tests[i].name);
        bool success = false;
        try {
            success = tests[i].func(&state, report);
        } catch (const std::runtime_error &ex) { /* Empty */ }
        if (success) {
            succeeded++;
            if (report)
                std::fprintf(report, "test '%s' success\n\n", tests[i].name);
        } else {
            std::fprintf(STREAM, PRE BOLDRED "| %s" RESET " failed (%s @ %d)\n",
                         tests[i].name, tests[i].file, tests[i].line);
            if (report)
                std::fprintf(report, "test '%s' failed\n\n", tests[i].name);
        }
    }
    std::fprintf(STREAM, PRE);

    std::fprintf(STREAM, GREEN "Passed: " RESET "%d\n", succeeded);
    if (succeeded != num_tests)
        std::fprintf(STREAM, RED "Failed: " RESET "%d\n", num_tests - succeeded);

    if (report)
        std::fprintf(report, "Tests done: %d/%d passed\n", succeeded, num_tests);

#ifdef REPORT
    if (!report)
        std::fprintf(STREAM, RED "Unable to write report\n" RESET);
    else
        std::fclose(report);
#endif
    delete[] tests;

    return num_tests - succeeded;
}

