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
#define STR(x) #x
#define ASSERT(pass, msg) _smek_assert(__FILE__, __LINE__, pass, msg, STR(pass))
void _smek_assert(const char *file, u32 line, bool passed, const char *msg, const char *expr);
#define CHECK(pass, msg) _smek_check(__FILE__, __LINE__, pass, msg, STR(pass))
bool _smek_check(const char *file, u32 line, bool passed, const char *msg, const char *expr);
