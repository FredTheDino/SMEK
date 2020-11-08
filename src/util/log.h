#pragma once
#include <stdexcept>
#include "../math/types.h"
#include "color.h"

namespace LogLevel {
static const u32 NONE    = 0;
static const u32 TRACE   = 1 << 0;
static const u32 INFO    = 1 << 1;
static const u32 WARNING = 1 << 2;
static const u32 ERROR   = 1 << 3;
static const u32 ALL     =(1 << 4) - 1; // update this if changing log levels
}

#define ERR(...)   _smek_log_err(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define WARN(...)  _smek_log_warn(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define INFO(...)  _smek_log_info(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define TRACE(...) _smek_log_trace(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define UNREACHABLE(...) _smek_unreachable(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define STR(x) #x

#define ASSERT(pass, ...) _smek_assert(__FILE__, __LINE__, __func__, pass, STR(pass), __VA_ARGS__)
#define CHECK(pass, ...)  _smek_check(__FILE__, __LINE__, __func__, pass, STR(pass), __VA_ARGS__)

#define ASSERT_EQ(LHS, RHS)                                                                           \
    do {                                                                                              \
        auto lhs = LHS;                                                                               \
        decltype(lhs) rhs = RHS;                                                                      \
        ASSERT((lhs) == (rhs), STR(LHS) " ({}) and " STR(RHS) " ({}) should be equal", (lhs), (rhs)); \
    } while (false)

#define ASSERT_NE(LHS, RHS)                                                                               \
    do {                                                                                                  \
        auto lhs = LHS;                                                                                   \
        decltype(lhs) rhs = RHS;                                                                          \
        ASSERT((lhs) != (rhs), STR(LHS) " ({}) and " STR(RHS) " ({}) should not be equal", (lhs), (rhs)); \
    } while (false)

#define ASSERT_LT(LHS, RHS)                                                                          \
    do {                                                                                             \
        auto lhs = LHS;                                                                              \
        decltype(lhs) rhs = RHS;                                                                     \
        ASSERT((lhs) < (rhs), STR(LHS) " ({}) should be less than " STR(RHS) " ({})", (lhs), (rhs)); \
    } while (false)

#define ASSERT_GT(LHS, RHS)                                                                             \
    do {                                                                                                \
        auto lhs = LHS;                                                                                 \
        decltype(lhs) rhs = RHS;                                                                        \
        ASSERT((lhs) > (rhs), STR(LHS) " ({}) should be greater than " STR(RHS) " ({})", (lhs), (rhs)); \
    } while (false)

#define ASSERT_LE(LHS, RHS)                                                                                       \
    do {                                                                                                          \
        auto lhs = LHS;                                                                                           \
        decltype(lhs) rhs = RHS;                                                                                  \
        ASSERT((lhs) <= (rhs), STR(LHS) " ({}) should be less than or equal to " STR(RHS) " ({})", (lhs), (rhs)); \
    } while (false)

#define ASSERT_GE(LHS, RHS)                                                                                          \
    do {                                                                                                             \
        auto lhs = LHS;                                                                                              \
        decltype(lhs) rhs = RHS;                                                                                     \
        ASSERT((lhs) >= (rhs), STR(LHS) " ({}) should be greater than or equal to " STR(RHS) " ({})", (lhs), (rhs)); \
    } while (false)

void _smek_log(const char *buffer, u32 log_level);

template <typename... Args>
void _smek_log_err(const char *file, u32 line, const char *func, const char *message, Args... args);

template <typename... Args>
void _smek_log_warn(const char *file, u32 line, const char *func, const char *message, Args... args);

template <typename... Args>
void _smek_log_info(const char *file, u32 line, const char *func, const char *message, Args... args);

template <typename... Args>
void _smek_log_trace(const char *file, u32 line, const char *func, const char *message, Args... args);

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
    _smek_log(buffer, LogLevel::ERROR);
}

template <typename... Args>
void _smek_log_warn(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, YELLOW "W {}" RESET " @ {} ({}): ", file, line, func);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, message, args...);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n");
    _smek_log(buffer, LogLevel::WARNING);
}

template <typename... Args>
void _smek_log_info(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, WHITE "I {}" RESET " @ {} ({}): ", file, line, func);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, message, args...);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n");
    _smek_log(buffer, LogLevel::INFO);
}

template <typename... Args>
void _smek_log_trace(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, "D {} @ {} ({}): ", file, line, func);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, message, args...);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n");
    _smek_log(buffer, LogLevel::TRACE);
}

template <typename... Args>
void _smek_unreachable(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, RED "U {}" RESET " @ {} ({}) unreachable:\n", file, line, func);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, BOLDRED "| " RESET);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, message, args...);
    len += sntprint(buffer + len, LOG_BUFFER_SIZE - len, "\n" BOLDRED "|" RESET " Stacktrace:\n");
    _smek_log(buffer, LogLevel::ERROR);
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
    _smek_log(buffer, LogLevel::ERROR);
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
        _smek_log(buffer, LogLevel::WARNING);
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
TRACE(...)

///*
INFO(...)

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
