#pragma once
#include "../math/types.h"
#include "../math/smek_math.h"
#include <chrono>
#include <unordered_map>

namespace Performance {

constexpr u32 HISTORY_LENGTH = 1001;

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
struct Metric {
    const char *name;

    const char *func;
    const char *file;
    u32 line;

    u32 num_calls;
    u32 total_nano_seconds; // Wraps via overflow every ~4.3 seconds

    f32 total_hist[HISTORY_LENGTH];
    f32 time_per_hist[HISTORY_LENGTH];

    TimePoint start;
};
using MetricCollection = std::unordered_map<u64, Metric>;

u64 begin_time_block(const char *name,
                     u64 hash_uuid,
                     const char *func,
                     const char *file,
                     u32 line);

void end_time_block(u64 hash_uuid);

///*
// Capture performance data, either between
// the call of "capture_begin" and "capture_end",
// or for a number of frames by calling "capture_frames".
//
// If "capture_frames" is called in a capture, "n" more
// frames will be captured.
void capture_begin();
void capture_end();
void capture_frames(i32 n = 100);

///*
// Returns true when a capture is running.
bool capture_is_running();

///*
// Returns how many frames are left for the current
// capture. -1 means unkown/infinite.
i32 capture_frames_left();

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
#define PERFORMANCE(name)          _a_performance(name, __LINE__)
} // namespace Performance
