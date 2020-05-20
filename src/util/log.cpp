#include "log.h"
#include "color.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <stdexcept>
#include <execinfo.h>
#include <unistd.h>


void _smek_error_log(const char *file, u32 line, const char *func, const char *message, ...) {
    std::fprintf(stderr, RED "! %s" RESET "|%s|" BLUE "%d" RESET ": ", file, func, line);
    va_list args;
    va_start(args, message);
    std::vfprintf(stderr, message, args);
    va_end(args);
    std::fprintf(stderr, "\n");
}

void _smek_warn_log(const char *file, u32 line, const char *func, const char *message, ...) {
    std::fprintf(stderr, YELLOW "? %s" RESET "|%s|" BLUE "%d" RESET ": ", file, func, line);
    va_list args;
    va_start(args, message);
    std::vfprintf(stderr, message, args);
    va_end(args);
    std::fprintf(stderr, "\n");
}

#define HALT(msg) \
    {\
        void *array[128]; size_t size;\
        size = backtrace(array, sizeof(array) / sizeof(array[0]));\
        backtrace_symbols_fd(array, size, STDERR_FILENO);\
        throw std::runtime_error(msg);\
    }

void _smek_info_log(const char *file, u32 line, const char *func, const char *message, ...) {
    std::fprintf(stderr, GREEN " %s" RESET "|%s|" BLUE "%d" RESET ": ", file, func, line);
    va_list args;
    va_start(args, message);
    std::vfprintf(stderr, message, args);
    va_end(args);
    std::fprintf(stderr, "\n");
}

void _smek_unreachable(const char *file, u32 line, const char *func, const char *message, ...) {
    std::fprintf(stderr, RED "%s" RESET "|%s|" RED "%d" RESET ": Unreachable\n", file, func, line);
    va_list args;
    va_start(args, message);
    std::vfprintf(stderr, message, args);
    va_end(args);
    std::fprintf(stderr, "\n" BOLDRED "| " RESET " End of Transmission\n");

    HALT("Unreachable!");
}

void _smek_assert(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *msg, ...) {
    if (passed) return;

    std::fprintf(stderr, RED "%s" RESET "|%s|" RED "%d" RESET ": ASSERT(%s)\n", file, func, line, expr);
    std::fprintf(stderr, BOLDRED "| " RESET);
    va_list args;
    va_start(args, msg);
    std::vfprintf(stderr, msg, args);
    va_end(args);
    std::fprintf(stderr, "\n" BOLDRED "| " RESET "End of Transmission\n");

    HALT("Assert!");
}

bool _smek_check(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *msg, ...) {
    if (!passed) {
        std::fprintf(stderr, YELLOW "%s" RESET "|%s|" BLUE "%d" RESET ": %s\n", file, func, line, expr);
        std::fprintf(stderr, YELLOW "| " RESET);
        va_list args;
        va_start(args, msg);
        std::vfprintf(stderr, msg, args);
        va_end(args);
        std::fprintf(stderr, "\n");
    }
    return passed;
}
