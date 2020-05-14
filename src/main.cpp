#include <cstdio>

#include "test.h"

TEST_CASE("First", { return std::printf("Hello I am a test.\n"); });
TEST_CASE("Second", { return std::printf("Hello I am also a test\n"); });

int main() {
    std::printf("Hello world!\n");
#ifdef TESTS
    _global_tests.run();
    return 0;
#endif
    std::printf("Didn't run tests\n");
    return 0;
}

