#include <cstdio>

#include "test.h"

int main() {
    std::printf("Hello world!\n");
#ifdef TESTS
    _global_tests.run();
    return 0;
#endif
    std::printf("Didn't run tests\n");
    return 0;
}

