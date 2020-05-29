#pragma once
#include <stdexcept>
#include "../math/types.h"
#include "color.h"

// https://stackoverflow.com/a/5891370/4904628, info on ##__VA_ARGS__
#define ERROR(...) _smek_log_err(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define WARN(...) _smek_log_warn(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG(...) _smek_log_info(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define UNREACHABLE(msg, ...) _smek_unreachable(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)

#define STR(x) #x

#define ASSERT(pass, msg, ...) _smek_assert(__FILE__, __LINE__, __func__, pass, STR(pass), msg, ##__VA_ARGS__)
#define CHECK(pass, msg, ...) _smek_check(__FILE__, __LINE__, __func__, pass, STR(pass), msg, ##__VA_ARGS__)

template <typename... Args>
void _smek_log_err(const char *file, u32 line, const char *func, const char *message, Args... args);

template <typename... Args>
void _smek_log_warn(const char *file, u32 line, const char *func, const char *message, Args... args);

template <typename... Args>
void _smek_log_info(const char *file, u32 line, const char *func, const char *message, Args... args);

template <typename... Args>
void _smek_unreachable(const char *file, u32 line, const char *func, const char *message, Args... args);

template <typename... Args>
void _smek_assert(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *message, Args... args);

template <typename... Args>
bool _smek_check (const char *file, u32 line, const char *func, bool passed, const char *expr, const char *message, Args... args);

void print_stacktrace(unsigned int max_frames=63);

#include "tprint.h"

// These have to live here now, because of templates.
template <typename... Args>
void _smek_log_err(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[512] = {};
    u32 len = 0;
    len += sntprint(buffer, 512, RED "E {}" RESET " @ {} ({}): ", file, line, func);
    len += sntprint(buffer + len, 512 - len, message, args...);
    len += sntprint(buffer + len, 512 - len, "\n");
    smek_print(buffer);
}

template <typename... Args>
void _smek_log_warn(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[512] = {};
    u32 len = 0;
    len += sntprint(buffer, 512, YELLOW "W {}" RESET " @ {} ({}): ", file, line, func);
    len += sntprint(buffer + len, 512 - len, message, args...);
    len += sntprint(buffer + len, 512 - len, "\n");
    smek_print(buffer);
}

template <typename... Args>
void _smek_log_info(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[512] = {};
    u32 len = 0;
    len += sntprint(buffer, 512, WHITE "I {}" RESET " @ {} ({}): ", file, line, func);
    len += sntprint(buffer + len, 512 - len, message, args...);
    len += sntprint(buffer + len, 512 - len, "\n");
    smek_print(buffer);
}

template <typename... Args>
void _smek_unreachable(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[512] = {};
    u32 len = 0;
    len += sntprint(buffer, 512, RED "U {}" RESET " @ {} ({}) unreachable:\n", file, line, func);
    len += sntprint(buffer + len, 512 - len, BOLDRED "| " RESET);
    len += sntprint(buffer + len, 512 - len, message, args...);
    len += sntprint(buffer + len, 512 - len, "\n" BOLDRED "|" RESET " Stacktrace:\n");
    smek_print(buffer);
    print_stacktrace();

    throw std::runtime_error("Unreachable");
}

template <typename... Args>
void _smek_assert(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *message, Args... args) {
    if (passed) return;

    char buffer[512] = {};
    u32 len = 0;
    len += sntprint(buffer, 512, RED "A {}" RESET " @ {} ({}) assert({}):\n", file, line, func, expr);
    len += sntprint(buffer + len, 512 - len, BOLDRED "| " RESET);
    len += sntprint(buffer + len, 512 - len, message, args...);
    len += sntprint(buffer + len, 512 - len, "\n" BOLDRED "|" RESET " Stacktrace:\n");
    smek_print(buffer);
    print_stacktrace();

    throw std::runtime_error("Assert");
}

template <typename... Args>
bool _smek_check(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *msg, Args... args) {
    if (!passed) {
        char buffer[512] = {};
        u32 len = 0;
        len += sntprint(buffer, 512, YELLOW "C" RESET " {} @ {} ({}) check({}):\n", file, line, func, expr);
        len += sntprint(buffer + len, 512 - len, YELLOW "| " RESET);
        len += sntprint(buffer + len, 512 - len, msg, args...);
        len += sntprint(buffer + len, 512 - len, "\n");
        smek_print(buffer);
    }
    return passed;
}

#if 0

///# Logging
//
// Different ways of logging and checking values. All macros support
// printf-like formats and arguments. Most functions (but not all) require at
// least a string to be printed. The output is always on stderr.

///*
LOG(...)

///*
WARN(...)

///*
ERROR(...)

///*
// Message is optional. Warns if <code>func</code> is evaluated to false.
CHECK(func, ...)

///*
// Message is optional. Errors if <code>func</code> is evaluated to false.
ASSERT(func, ...)

///*
UNREACHABLE(...)

#endif
