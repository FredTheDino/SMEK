#include <stdexcept>
#include <cstdlib>

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
        if ARGUMENT ("--help", "-h") {
            //TODO(gu)
            std::printf("Usage: tests [--help] [--ci] [--report] [--report-path <path>]\n");
            return 0;
        } else if ARGUMENT ("--ci", "-c") {
            ci = true;
        } else if ARGUMENT ("--report", "-r") {
            write_report = true;
        } else if ARGUMENT ("--report-path", "-p") {
            write_report = true;
            report_path = argv[++index];
        } else {
            ERR("Unknown command line argument '{}'", argv[index]);
        }
    }
#undef ARGUMENT
    return _global_tests.run(ci, write_report, report_path);
}

void init_tests_state(GameState *tests_state ) {
    tests_state->entity_system.m_client_id = SDL_CreateMutex();
}

#define STREAM stderr

unsigned int TestSuite::run(bool ci, bool write_report, const char *path) {
    FILE *report;
    if (write_report) {
        report = std::fopen(path, "w");
        if (!report) {
            ftprint(STREAM, "Unable to open report\n");
        }
    } else {
        report = nullptr;
    }

    const char *PRE, *POST;
    if (ci) {
        PRE = "";
        POST = "\n";
    } else {
#ifdef COLOR_DISABLE
        PRE = "";
        POST = "\n";
#else
        PRE = CLEAR "\r";
        POST = "\r";
#endif
    }

    std::fprintf(STREAM, "Running %d tests\n", num_tests);
    unsigned int succeeded = 0;

    for (unsigned int i = 0; i < num_tests; i++) {
        GameState state = {};
        init_tests_state(&state);
        _test_gs = &state;
        // remember to increase the 03 here if we pass 999 tests
        ftprint(STREAM, "{}{03}: {}{}", PRE, i + 1, tests[i].name, POST);
        LOG_TESTS("{03}: {}", i + 1, tests[i].name);
        bool success = false;
        try {
            success = tests[i].func(&state, report);
        } catch (const std::runtime_error &ex) { /* Empty */
        }
        if (success) {
            succeeded++;
            LOG_TESTS("'{}' succeeded at {} @ {}\n", tests[i].name, tests[i].file, tests[i].line);
        } else {
            ftprint(STREAM, "{}" BOLDRED "| {}" RESET " failed ({} @ {})\n",
                    PRE, tests[i].name, tests[i].file, tests[i].line);
            LOG_TESTS("'{}' failed at {} @ {}\n", tests[i].name, tests[i].file, tests[i].line);
        }
    }
    ftprint(STREAM, "{}", PRE);

    ftprint(STREAM, GREEN "Passed: " RESET "{}\n", succeeded);
    if (succeeded != num_tests)
        ftprint(STREAM, RED "Failed: " RESET "{}\n", num_tests - succeeded);

    LOG_TESTS("Tests done: {}/{} passed", succeeded, num_tests);

    if (report)
        std::fclose(report);
    delete[] tests;

    return num_tests - succeeded;
}
