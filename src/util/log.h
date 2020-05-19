#pragma once
#include "../math/types.h"

#define ERROR(...) _smek_error_log(__FILE__, __LINE__, __VA_ARGS__)
void _smek_error_log(const char *file, u32 line, const char *message, ...);

#define WARN(...) _smek_warn_log(__FILE__, __LINE__, __VA_ARGS__)
void _smek_warn_log(const char *file, u32 line, const char *message, ...);

#define LOG(...) _smek_info_log(__FILE__, __LINE__, __VA_ARGS__)
void _smek_info_log(const char *file, u32 line, const char *message, ...);


#define UNREACHABLE _smek_unreachable(__FILE__, __LINE__)
void _smek_unreachable(const char *file, u32 line);

// https://stackoverflow.com/a/5891370/4904628
// Tested with clang++ as well.

#define STR(x) #x
#define ASSERT(pass, msg, ...) _smek_assert(__FILE__, __LINE__, pass, STR(pass), msg, ##__VA_ARGS__)
void _smek_assert(const char *file, u32 line, bool passed, const char *expr, const char *msg, ...);
#define CHECK(pass, msg, ...) _smek_check(__FILE__, __LINE__, pass, STR(pass), msg, ##__VA_ARGS__)
bool _smek_check(const char *file, u32 line, bool passed, const char *expr, const char *msg, ...);
