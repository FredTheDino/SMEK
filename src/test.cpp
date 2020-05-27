#include <cstdio>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

#include "util/color.h"

#include "test.h"

static GameState *_test_gs;
GameState *GAMESTATE() { return _test_gs; }

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

int main(int argc, char **argv) { // Test entry point
#define ARGUMENT(LONG, SHORT) (std::strcmp((LONG), argv[index]) == 0 || std::strcmp((SHORT), argv[index]) == 0)
    bool ci = false;
    bool write_report = false;
    const char *report_path = "report.txt";
    for (int index = 1; index < argc; index++) {
        if ARGUMENT("--help", "-h") {
            //TODO(gu)
            std::printf("Usage: tests [--help] [--ci] [--report] [--report-path <path>]\n");
            return 0;
        } else if ARGUMENT("--ci", "-c") {
            ci = true;
        } else if ARGUMENT("--report", "-r") {
            write_report = true;
        } else if ARGUMENT("--report-path", "-p") {
            write_report = true;
            report_path = argv[++index];
        } else {
            ERROR("Unknown command line argument '%s'", argv[index]);
        }
    }
#undef ARGUMENT
    return _global_tests.run(ci, write_report, report_path);
}

#define STREAM stderr

unsigned int TestSuite::run(bool ci, bool write_report, const char *path) {
    FILE *report;
    if (write_report) {
        report = std::fopen(path, "w");
        if (!report) {
            std::fprintf(STREAM, "Unable to open report\n");
        }
    } else {
        report = nullptr;
    }
    
    const char *PRE, *POST;
    if (ci) {
        PRE = "";
        POST = "\n";
    } else {
        PRE = CLEAR "\r";
        POST = "\r";
    }

    std::fprintf(STREAM, "Running %d tests\n", num_tests);
    unsigned int succeeded = 0;

    for (unsigned int i = 0; i < num_tests; i++) {
        GameState state = {};
        _test_gs = &state;
        std::fprintf(STREAM, "%s%02d: %s%s", PRE, i+1, tests[i].name, POST);
        LOG_TESTS("%02d: %s", i+1, tests[i].name);
        bool success = false;
        try {
            success = tests[i].func(&state, report);
        } catch (const std::runtime_error &ex) { /* Empty */ }
        if (success) {
            succeeded++;
            LOG_TESTS("'%s' succeeded at %s @ %d\n", tests[i].name, tests[i].file, tests[i].line);
        } else {
            std::fprintf(STREAM, "%s" BOLDRED "| %s" RESET " failed (%s @ %d)\n",
                         PRE, tests[i].name, tests[i].file, tests[i].line);
            LOG_TESTS("'%s' failed at %s @ %d\n", tests[i].name, tests[i].file, tests[i].line);
        }
    }
    std::fprintf(STREAM, "%s", PRE);

    std::fprintf(STREAM, GREEN "Passed: " RESET "%d\n", succeeded);
    if (succeeded != num_tests)
        std::fprintf(STREAM, RED "Failed: " RESET "%d\n", num_tests - succeeded);

    LOG_TESTS("Tests done: %d/%d passed", succeeded, num_tests);

    if (report)
        std::fclose(report);
    delete[] tests;

    return num_tests - succeeded;
}

