#pragma once
#include "../math/types.h"
#include "color.h"

#define ERROR(...) _smek_error_log(__FILE__, __LINE__, __VA_ARGS__)
void _smek_error_log(const char *file, u32 line, const char *message, ...);

#define LOG(...) _smek_info_log(__FILE__, __LINE__, __VA_ARGS__)
void _smek_info_log(const char *file, u32 line, const char *message, ...);

