#pragma once
#include "../math/types.h"
#include "../math/smek_math.h"
#include <chrono>
#include <unordered_map>

namespace Performance {

constexpr u32 HISTORY_LENGTH = 1001;
const f32 NANO_TO_MS = 1e-6;

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
    u32 num_calls_hist[HISTORY_LENGTH];

    TimePoint start;

    void new_frame(u32 frame) {
        total_hist[frame] = NANO_TO_MS * total_nano_seconds;
        time_per_hist[frame] = NANO_TO_MS * total_nano_seconds / (num_calls ?: 1);
        num_calls_hist[frame] = num_calls;
        num_calls = 0;
        total_nano_seconds = 0;
    }
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

///*
// A macro that creates a new performance counter for the
// rest of the block.
#define PERFORMANCE(name) _a_performance(name, __LINE__)

///*
// Returns true if the capture is currently enabled.
bool should_capture();

///*
// Writs a string to the capture file, only if the capture
// file is open and available, otherwise does nothing.
void write_to_capture_file(u32 size, const char *buf);

#define JSON_ARG(name) \
    "\"" STR(name) "\":\"{}\","

#define _b_capture(line, ...)                                                                            \
    if (Performance::should_capture()) {                                                                 \
        char buffer[256];                                                                                \
        u32 size = sntprint(buffer, LEN(buffer),                                                         \
                            R"(,%{"cat":"CAPTURE","tid":"{}","ts":{},"name":"{}:{}")"                    \
                            R"(,"pid":0,"ph":"B","s":"g")"                                               \
                            ",\"args\":%{" MAP(JSON_ARG, func, file, line, __VA_ARGS__) "\"-\":\"-\"}}", \
                            SDL_ThreadID(),                                                              \
                            Performance::Clock::now().time_since_epoch().count() / 1000.0,               \
                            __func__, STR(line),                                                         \
                            __func__, __FILE__, __LINE__, __VA_ARGS__);                                  \
        Performance::write_to_capture_file(size, buffer);                                                \
    }                                                                                                    \
    defer {                                                                                              \
        if (Performance::should_capture) {                                                               \
            char buffer[256];                                                                            \
            u32 size = sntprint(buffer, LEN(buffer), R"(,%{"ph":"E","pid":0,"tid":{},"ts":{}})",         \
                                SDL_ThreadID(),                                                          \
                                Performance::Clock::now().time_since_epoch().count() / 1000.0);          \
            Performance::write_to_capture_file(size, buffer);                                            \
        }                                                                                                \
    }

#define _a_capture(line, ...) _b_capture(line, __VA_ARGS__)

///*
// Writes the varaibles passed as arguments to the capture file,
// and records the time for the remainder of the block.
#define CAPTURE(...) _a_capture(__LINE__, __VA_ARGS__)

} // namespace Performance
