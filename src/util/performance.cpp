#include "performance.h"
#include "log.h"
#include <unordered_map>
#include "../game.h"

#ifdef IMGUI_ENABLE
#include "imgui/implot.h"
#endif

namespace Performance {

#ifdef PERFORMANCE_ENABLE
struct PerformanceMetrics {
    // TODO(ed): Locks and working with different threads.
    std::unordered_map<u64, PerformanceCounter> metrics;
    u32 frame = 0;
} gpc;

const TimePoint null_time = {};

u64 begin_time_block(const char *name,
                     u64 hash_uuid,
                     const char *func,
                     const char *file,
                     u32 line) {
    if (!gpc.metrics.contains(hash_uuid)) {
        PerformanceCounter counter = {};
        counter.name = name;
        counter.function = func;
        counter.file = file;
        counter.line = line;
        gpc.metrics[hash_uuid] = counter;
    }

    PerformanceCounter &ref = gpc.metrics[hash_uuid];
    ASSERT(ref.start == null_time, "Performance block started twice!");
    ref.start = std::chrono::high_resolution_clock::now();
    return hash_uuid;
}

void end_time_block(u64 hash_uuid) {
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::nanoseconds;
    PerformanceCounter &ref = gpc.metrics[hash_uuid];
    ref.num_calls += 1;
    TimePoint start = ref.start;
    TimePoint end = high_resolution_clock::now();
    ref.start = null_time;
    ref.total_nano_seconds += duration_cast<nanoseconds>(end - start).count();
}

#ifdef IMGUI_ENABLE
#define NANO_TO_MS 1e-6
void report() {
    if (!GAMESTATE()->imgui.performance_enabled) return;
    ImGui::Begin("Performance");
    gpc.frame += 1;
    gpc.frame %= HISTORY_LENGTH;
    ImPlot::SetNextPlotLimits(0, HISTORY_LENGTH, 0, 16);
    if (ImPlot::BeginPlot("Total Time", "Frame", "Time (ms)", Vec2(-1, 0), ImPlotFlags_None, ImPlotAxisFlags_Lock, ImPlotAxisFlags_Lock)) {
        for (auto &[hash, counter] : gpc.metrics) {
            counter.total_hist[gpc.frame] = NANO_TO_MS * counter.total_nano_seconds;
            ImPlot::PlotLine(counter.name, counter.total_hist, HISTORY_LENGTH);
        }
        ImPlot::EndPlot();
    }

    ImPlot::SetNextPlotLimits(0, HISTORY_LENGTH, 0, 16);
    if (ImPlot::BeginPlot("Time Per Call", "Frame", "Time (ms)", Vec2(-1, 0), ImPlotFlags_None, ImPlotAxisFlags_Lock, ImPlotAxisFlags_Lock)) {
        for (auto &[hash, counter] : gpc.metrics) {
            counter.time_per_hist[gpc.frame] = NANO_TO_MS * counter.total_nano_seconds / (counter.num_calls ?: 1);
            ImPlot::PlotLine(counter.name, counter.time_per_hist, HISTORY_LENGTH);
        }
        ImPlot::EndPlot();
    }
    for (auto &[hash, counter] : gpc.metrics) {
        counter.num_calls = 0;
        counter.total_nano_seconds = 0;
    }
    ImGui::End();
}
#else // Without IMGUI
void report() {
    for (auto &[hash, counter] : gpc.metrics) {
        LOG("{} - #{} {}ns {}ns/call",
            counter.name,
            counter.num_calls,
            counter.total_nano_seconds,
            counter.total_nano_seconds / (counter.num_calls ?: 1));
        counter.num_calls = 0;
        counter.total_nano_seconds = 0;
    }
}
#endif

#else
u64 begin_time_block(const char *name,
                     u64 hash_uuid,
                     const char *func,
                     const char *file,
                     u32 line) {}
void end_time_block(u64 hash_uuid) {}
void report() {}
#endif
}
