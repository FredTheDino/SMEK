#include "log.h"
#include <cstdio>
#include <cstdarg>

void _smek_error_log(const char *file, u32 line, const char *message, ...) {
    std::fprintf(stderr, YELLOW "!%s" RESET "|" BLUE "%d" RESET ": ", file, line);
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


