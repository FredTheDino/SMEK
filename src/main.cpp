#include <cstdio>
#include "test.h"

TEST_CASE("First test of tests", { return std::printf("Nice, it did what it should!6\n"); });
#if 1
TEST_CASE("Second test of tests", { return std::printf("Nice, it did what it should!3\n"); });
TEST_CASE("Third test of tests", { return std::printf("Nice, it did what it should!2\n"); });
TEST_CASE("Fourth test of tests", { return std::printf("Nice, it did what it should!1\n"); });
#endif

int main() {
    std::printf("Hello world!\n");
    _global_tests.run();
    return 0;
}

