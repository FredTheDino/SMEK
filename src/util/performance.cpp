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

    TimePoint frame_start = {};
    f32 frame_time[HISTORY_LENGTH] = {};
    f32 average_frame_time[HISTORY_LENGTH] = {};
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

#define DRAW_NOW_LINE                                  \
    do {                                               \
        f32 xs[] = { (f32)gpc.frame, (f32)gpc.frame }; \
        f32 ys[] = { -10, 30 };                        \
        ImPlot::PlotLine("NOW", xs, ys, 2);            \
    } while (false)

    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::nanoseconds;
    TimePoint now = high_resolution_clock::now();
    f32 frame_time = duration_cast<nanoseconds>(now - gpc.frame_start).count() * NANO_TO_MS;
    frame_time = frame_time < 40 ? frame_time : 40;
    gpc.frame_start = now;

    u32 prev_frame = gpc.frame;
    gpc.frame += 1;
    gpc.frame %= HISTORY_LENGTH;
    ImPlot::SetNextPlotLimits(0, HISTORY_LENGTH, 0, 16);

    ImPlot::SetNextPlotLimits(0, HISTORY_LENGTH, 0, 20);
    if (ImPlot::BeginPlot("Total Frame Time",
                          "Frame",
                          "Time (ms)",
                          Vec2(-1, 0),
                          ImPlotFlags_None,
                          ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoDecorations,
                          ImPlotAxisFlags_Lock)) {

        DRAW_NOW_LINE;
        f32 prev_average = gpc.average_frame_time[prev_frame];
        f32 oldest_frame_time = gpc.frame_time[gpc.frame];
        f32 delta_frame_time = frame_time - oldest_frame_time;
        f32 new_average = prev_average + delta_frame_time / HISTORY_LENGTH;
        gpc.average_frame_time[gpc.frame] = new_average;
        gpc.frame_time[gpc.frame] = frame_time;
        ImPlot::PlotLine("Raw Frametime", gpc.frame_time, HISTORY_LENGTH);
        ImPlot::PlotLine("Average Frametime", gpc.average_frame_time, HISTORY_LENGTH);
        ImPlot::EndPlot();
    }

    if (ImPlot::BeginPlot("Total Time",
                          "Frame",
                          "Time (ms)",
                          Vec2(-1, 0),
                          ImPlotFlags_None,
                          ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoDecorations,
                          ImPlotAxisFlags_Lock)) {
        DRAW_NOW_LINE;
        for (auto &[hash, counter] : gpc.metrics) {
            counter.total_hist[gpc.frame] = NANO_TO_MS * counter.total_nano_seconds;
            ImPlot::PlotLine(counter.name, counter.total_hist, HISTORY_LENGTH);
        }
        ImPlot::EndPlot();
    }

    ImPlot::SetNextPlotLimits(0, HISTORY_LENGTH, 0, 16);
    if (ImPlot::BeginPlot("Time Per Call",
                          "Frame",
                          "Time (ms)",
                          Vec2(-1, 0),
                          ImPlotFlags_None,
                          ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoDecorations,
                          ImPlotAxisFlags_Lock)) {
        DRAW_NOW_LINE;
        for (auto &[hash, counter] : gpc.metrics) {
            counter.time_per_hist[gpc.frame] = NANO_TO_MS * counter.total_nano_seconds / (counter.num_calls ?: 1);
            ImPlot::PlotLine(counter.name, counter.time_per_hist, HISTORY_LENGTH);
        }
        ImPlot::EndPlot();
    }

    ImPlot::SetNextPlotLimits(0, 1, 0, 1, ImGuiCond_Always);
    if (ImPlot::BeginPlot("Number of calls",
                          nullptr,
                          nullptr,
                          Vec2(250, 250),
                          ImPlotFlags_NoMousePos,
                          ImPlotAxisFlags_NoDecorations,
                          ImPlotAxisFlags_NoDecorations)) {
        const u32 MAX_NUM_PIE_PARTS = 128;
        const char *labels[MAX_NUM_PIE_PARTS];
        u32 calls[MAX_NUM_PIE_PARTS];
        u32 i = 0;
        u32 total = 0;
        for (auto &[hash, counter] : gpc.metrics) {
            if (i >= MAX_NUM_PIE_PARTS) break;
            labels[i] = counter.name;
            calls[i] = counter.num_calls;
            total += counter.num_calls;
            i++;
        }
        ImPlot::PlotPieChart(labels, calls, i, 0.5, 0.5, 0.4, true);
        ImPlot::EndPlot();
        ImGui::Text("Total: %d", total);
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
