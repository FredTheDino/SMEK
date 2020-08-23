#pragma once
#include <stdexcept>
#include "../math/types.h"
#include "color.h"

// https://stackoverflow.com/a/5891370/4904628, info on ##__VA_ARGS__
#define ERR(...)              _smek_log_err(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define WARN(...)             _smek_log_warn(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG(...)              _smek_log_info(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define UNREACHABLE(msg, ...) _smek_unreachable(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)

#define STR(x) #x

#define ASSERT(pass, msg, ...) _smek_assert(__FILE__, __LINE__, __func__, pass, STR(pass), msg, ##__VA_ARGS__)
#define CHECK(pass, msg, ...)  _smek_check(__FILE__, __LINE__, __func__, pass, STR(pass), msg, ##__VA_ARGS__)

#define ASSERT_EQ(LHS, RHS) ASSERT((LHS) == (RHS), STR(LHS) " ({}) and " STR(RHS) " ({}) should be equal", (LHS), (RHS));
#define ASSERT_NE(LHS, RHS) ASSERT((LHS) != (RHS), STR(LHS) " ({}) and " STR(RHS) " ({}) should not be equal", (LHS), (RHS));
#define ASSERT_LT(LHS, RHS) ASSERT((LHS) < (RHS), STR(LHS) " ({}) should be less than " STR(RHS) " ({})", (LHS), (RHS));
#define ASSERT_GT(LHS, RHS) ASSERT((LHS) > (RHS), STR(LHS) " ({}) should be greater than " STR(RHS) " ({})", (LHS), (RHS));
#define ASSERT_LE(LHS, RHS) ASSERT((LHS) <= (RHS), STR(LHS) " ({}) should be less than or equal to " STR(RHS) " ({})", (LHS), (RHS));
#define ASSERT_GE(LHS, RHS) ASSERT((LHS) >= (RHS), STR(LHS) " ({}) should be greater than or equal to " STR(RHS) " ({})", (LHS), (RHS));

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
bool _smek_check(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *message, Args... args);

void print_stacktrace(unsigned int max_frames = 63);

#include "tprint.h"

const u32 LOG_BUFFER_SIZE = 512;

// These have to live here now, because of templates.
template <typename... Args>
void _smek_log_err(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, RED "E {}" RESET " @ {} ({}): ", file, line, func);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, message, args...);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n");
    smek_print(buffer);
}

template <typename... Args>
void _smek_log_warn(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, YELLOW "W {}" RESET " @ {} ({}): ", file, line, func);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, message, args...);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n");
    smek_print(buffer);
}

template <typename... Args>
void _smek_log_info(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, WHITE "I {}" RESET " @ {} ({}): ", file, line, func);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, message, args...);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n");
    smek_print(buffer);
}

template <typename... Args>
void _smek_unreachable(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, RED "U {}" RESET " @ {} ({}) unreachable:\n", file, line, func);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, BOLDRED "| " RESET);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, message, args...);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n" BOLDRED "|" RESET " Stacktrace:\n");
    smek_print(buffer);
    print_stacktrace();

    throw std::runtime_error("Unreachable");
}

template <typename... Args>
void _smek_assert(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *message, Args... args) {
    if (passed) return;

    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, RED "A {}" RESET " @ {} ({}) assert({}):\n", file, line, func, expr);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, BOLDRED "| " RESET);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, message, args...);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n" BOLDRED "|" RESET " Stacktrace:\n");
    smek_print(buffer);
    print_stacktrace();

    throw std::runtime_error("Assert");
}

template <typename... Args>
bool _smek_check(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *msg, Args... args) {
    if (!passed) {
        char buffer[LOG_BUFFER_SIZE] = {};
        u32 len = 0;
        len += sntprint(buffer, LOG_BUFFER_SIZE, YELLOW "C" RESET " {} @ {} ({}) check({}):\n", file, line, func, expr);
        len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, YELLOW "| " RESET);
        len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, msg, args...);
        len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n");
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
ERR(...)

///*
// Message is optional. Warns if <code>func</code> is evaluated to false.
CHECK(func, ...)

///*
// Message is optional. Errors if <code>func</code> is evaluated to false.
ASSERT(func, ...)

///* Specialized asserts
ASSERT_EQ(LHS, RHS)
ASSERT_NE(LHS, RHS)
ASSERT_LT(LHS, RHS)
ASSERT_GT(LHS, RHS)
ASSERT_LE(LHS, RHS)
ASSERT_GE(LHS, RHS)

///*
UNREACHABLE(...)

#endif
