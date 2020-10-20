#pragma once
#include "../math/types.h"
#include "../math/smek_math.h"
#include <chrono>

namespace Performance {

constexpr u32 HISTORY_LENGTH = 1001;

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
struct PerformanceCounter {
    const char *name;

    const char *function;
    const char *file;
    u32 line;

    u32 num_calls;
    u32 total_nano_seconds; // This is around 4 seconds, should be enough.

    f32 total_hist[HISTORY_LENGTH];
    f32 time_per_hist[HISTORY_LENGTH];

    TimePoint start;
};

u64 begin_time_block(const char *name,
                     u64 hash_uuid,
                     const char *func,
                     const char *file,
                     u32 line);

void end_time_block(u64 hash_uuid);

void report();

#define _b_performance(name, line)                                  \
    auto _PERFORMANCE_BLOCK_##line = Performance::begin_time_block( \
        name,                                                       \
        hash(name __FILE__ STR(__LINE__)),                          \
        __func__,                                                   \
        __FILE__,                                                   \
        __LINE__);                                                  \
    defer { Performance::end_time_block(_PERFORMANCE_BLOCK_##line); }

#define _a_performance(name, line) _b_performance(name, line)
#define PERFORMACE(name)           _a_performance(name, __LINE__)
}
