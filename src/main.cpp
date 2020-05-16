#include <cstdio>

#include "test.h"

TEST_CASE("Zero", { return 0; });
TEST_CASE("One", { return 1; });
TEST_CASE("Two", { return 0; });

int main() {
    std::printf("Hello world!\n");
#ifdef TESTS
    _global_tests.run();
    return 0;
#endif
    std::printf("Didn't run tests\n");
    return 0;
}

