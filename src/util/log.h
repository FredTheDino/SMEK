#pragma once
#include <stdexcept>
#include "../math/types.h"
#include "color.h"

enum class LogLevel {
    TRACE,
    INFO,
    WARNING,
    ERROR,
    NOTHING,
};

#define _LOGLEVEL(LEVEL, FUNC, ...)                             \
    do {                                                        \
        if (GAMESTATE()->lowest_log <= (LEVEL))                 \
            FUNC(__FILE__, __LINE__, __func__, __VA_ARGS__);   \
    } while (0)

#define ERR(...)   _LOGLEVEL(LogLevel::ERROR, _smek_log_err, __VA_ARGS__)
#define WARN(...)  _LOGLEVEL(LogLevel::WARNING, _smek_log_warn, __VA_ARGS__)
#define INFO(...)  _LOGLEVEL(LogLevel::INFO, _smek_log_info, __VA_ARGS__)
#define TRACE(...) _LOGLEVEL(LogLevel::TRACE, _smek_log_trace, __VA_ARGS__)

#define ERR_ALWAYS(...)   _smek_log_err(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define WARN_ALWAYS(...)  _smek_log_warn(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define INFO_ALWAYS(...)  _smek_log_info(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define TRACE_ALWAYS(...) _smek_log_trace(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define UNREACHABLE(...) _smek_unreachable(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define STR(x) #x

#define ASSERT(pass, msg, ...) _smek_assert(__FILE__, __LINE__, __func__, pass, STR(pass), msg, ##__VA_ARGS__)
#define CHECK(pass, msg, ...)  _smek_check(__FILE__, __LINE__, __func__, pass, STR(pass), msg, ##__VA_ARGS__)

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
void _smek_log_trace(const char *file, u32 line, const char *func, const char *message, Args... args) {
    char buffer[LOG_BUFFER_SIZE] = {};
    u32 len = 0;
    len += sntprint(buffer, LOG_BUFFER_SIZE, "D {} @ {} ({}): ", file, line, func);
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
