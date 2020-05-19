#include "log.h"
#include "color.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

void _smek_error_log(const char *file, u32 line, const char *message, ...) {
    std::fprintf(stderr, RED "! %s" RESET "|" BLUE "%d" RESET ": ", file, line);
    va_list args;
    va_start(args, message);
    std::vfprintf(stderr, message, args);
    va_end(args);
    std::fprintf(stderr, "\n");
}

void _smek_warn_log(const char *file, u32 line, const char *message, ...) {
    std::fprintf(stderr, YELLOW "? %s" RESET "|" BLUE "%d" RESET ": ", file, line);
    va_list args;
    va_start(args, message);
    std::vfprintf(stderr, message, args);
    va_end(args);
    std::fprintf(stderr, "\n");
}


void _smek_info_log(const char *file, u32 line, const char *message, ...) {
    std::fprintf(stderr, GREEN " %s" RESET "|" BLUE "%d" RESET ": ", file, line);
    va_list args;
    va_start(args, message);
    std::vfprintf(stderr, message, args);
    va_end(args);
    std::fprintf(stderr, "\n");
}

void _smek_unreachable(const char *file, u32 line) {
    std::fprintf(stderr, RED "%s" RESET "|" RED "%d" RESET ": Unreachable\n", file, line);
    std::fprintf(stderr, BOLDRED "| " RESET " End of Transmission\n");

    std::exit('U'); // U for Unreachable
}

void _smek_assert(const char *file, u32 line, bool passed, const char *expr, const char *msg, ...) {
    if (passed) return;

    std::fprintf(stderr, RED "%s" RESET "|" RED "%d" RESET ": ASSERT(%s)\n", file, line, expr);
    std::fprintf(stderr, BOLDRED "| " RESET);
    va_list args;
    va_start(args, msg);
    std::vfprintf(stderr, msg, args);
    va_end(args);
    std::fprintf(stderr, "\n" BOLDRED "| " RESET "End of Transmission\n");

    std::exit('A'); // A for Assert
}

bool _smek_check(const char *file, u32 line, bool passed, const char *expr, const char *msg, ...) {
    if (!passed) {
        std::fprintf(stderr, YELLOW "%s" RESET "|" BLUE "%d" RESET ": %s\n", file, line, expr);
        std::fprintf(stderr, YELLOW "| " RESET);
        va_list args;
        va_start(args, msg);
        std::vfprintf(stderr, msg, args);
        va_end(args);
    }
    return passed;
}
