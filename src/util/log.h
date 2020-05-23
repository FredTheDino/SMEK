#pragma once
#include "../math/types.h"

#define ERROR(...) _smek_log_err(__FILE__, __LINE__, __func__, __VA_ARGS__)
void _smek_log_err(const char *file, u32 line, const char *func, const char *message, ...);

#define WARN(...) _smek_log_warn(__FILE__, __LINE__, __func__, __VA_ARGS__)
void _smek_log_warn(const char *file, u32 line, const char *func, const char *message, ...);

#define LOG(...) _smek_log_info(__FILE__, __LINE__, __func__, __VA_ARGS__)
void _smek_log_info(const char *file, u32 line, const char *func, const char *message, ...);

// https://stackoverflow.com/a/5891370/4904628

#define UNREACHABLE(msg, ...) _smek_unreachable(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
void _smek_unreachable(const char *file, u32 line, const char *func, const char *msg, ...);

#define STR(x) #x

#define ASSERT(pass, msg, ...) _smek_assert(__FILE__, __LINE__, __func__, pass, STR(pass), msg, ##__VA_ARGS__)
void _smek_assert(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *msg, ...);

#define CHECK(pass, msg, ...) _smek_check(__FILE__, __LINE__, __func__, pass, STR(pass), msg, ##__VA_ARGS__)
bool _smek_check(const char *file, u32 line, const char *func, bool passed, const char *expr, const char *msg, ...);
