#include "performance.h"
#include "log.h"
#include "../game.h"

#ifdef IMGUI_ENABLE
#include "imgui/implot.h"
#endif

namespace Performance {

#ifdef PERFORMANCE_ENABLE
thread_local SDL_mutex *metrics_thread_lock = nullptr;
thread_local MetricCollection metrics;

const TimePoint NULL_TIME = {};

static void register_thread_for_performance_counting() {
    SDL_LockMutex(GAMESTATE()->performance_list_lock);
    defer { SDL_UnlockMutex(GAMESTATE()->performance_list_lock); };

    metrics_thread_lock = SDL_CreateMutex();
    GAMESTATE()->perf_states.push_back({
        SDL_ThreadID(),
        SDL_GetThreadName(nullptr), // TODO(ed): This isn't working, no name is given.
        metrics_thread_lock,
        &metrics,
    });
}

static void unregister_thread_for_performance_counting() {
    SDL_LockMutex(GAMESTATE()->performance_list_lock);
    defer { SDL_UnlockMutex(GAMESTATE()->performance_list_lock); };

    for (auto it = GAMESTATE()->perf_states.begin();
         it != GAMESTATE()->perf_states.end();
         ++it) {
        if (it->id == SDL_threadID()) {
            GAMESTATE()->perf_states.erase(it);
            break;
        }
    }
    SDL_DestroyMutex(metrics_thread_lock);
}

// Automatically remove the element from the list
// when the thread is killed.
thread_local defer_expl[]() { unregister_thread_for_performance_counting(); };

#define LOCK() \
    SDL_LockMutex(metrics_thread_lock)

#define UNLOCK() \
    SDL_UnlockMutex(metrics_thread_lock)

u64 begin_time_block(const char *name,
                     u64 hash_uuid,
                     const char *func,
                     const char *file,
                     u32 line) {
    if (!metrics_thread_lock) {
        register_thread_for_performance_counting();
    }

    LOCK();
    bool contains = metrics.contains(hash_uuid);
    UNLOCK();

    if (!contains) {
        Metric counter = {};
        counter.name = name;
        counter.function = func;
        counter.file = file;
        counter.line = line;

        LOCK();
        metrics[hash_uuid] = counter;
        UNLOCK();
    }

    LOCK();
    Metric &ref = metrics[hash_uuid];
    ASSERT(ref.start == NULL_TIME, "Performance block started twice!");
    ref.start = Clock::now();
    UNLOCK();
    return hash_uuid;
}

f32 time_since(TimePoint a, TimePoint b) {
    using std::chrono::duration_cast;
    using std::chrono::nanoseconds;
    return duration_cast<nanoseconds>(b - a).count();
}

void end_time_block(u64 hash_uuid) {
    using std::chrono::duration_cast;
    using std::chrono::nanoseconds;
    TimePoint end = Clock::now();

    LOCK();
    Metric &ref = metrics[hash_uuid];
    ref.num_calls += 1;
    TimePoint start = ref.start;
    ref.start = NULL_TIME;
    ref.total_nano_seconds += time_since(start, end);
    UNLOCK();
}

#ifdef IMGUI_ENABLE
const f32 NANO_TO_MS = 1e-6;

TimePoint frame_start = {};
f32 frame_time[HISTORY_LENGTH] = {};
u32 frame = 0;

void report() {
    if (!GAMESTATE()->imgui.performance_enabled) return;
    ImGui::Begin("Performance");

    // The height of the graph plots, set to 20ms to make it easier
    // to see spikes and such.
    const f32 MAXIMUM_MS = 20;

#define DRAW_NOW_LINE                          \
    do {                                       \
        f32 xs[] = { (f32)frame, (f32)frame }; \
        f32 ys[] = { -10, 30 };                \
        ImPlot::PlotLine("Now", xs, ys, 2);    \
    } while (false)

    TimePoint now = Clock::now();
    if (frame_start.time_since_epoch().count() == 0) {
        frame_start = now;
    }
    f32 current_frame_time = time_since(frame_start, now) * NANO_TO_MS;
    frame_start = now;

    const i32 GRAPH_HEIGHT = 150;
    frame += 1;
    frame %= HISTORY_LENGTH;

    ImPlotStyle &style = ImPlot::GetStyle();
    style.PlotBorderSize = 0;
    style.LegendPadding = Vec2(0.0, 0.0);
    style.LegendInnerPadding = Vec2(5.0, 5.0);
    style.LegendSpacing = Vec2(2.0, 2.0);
    style.PlotPadding = Vec2();
    style.PlotBorderSize = 0.0;
    ImPlot::SetNextPlotLimits(0, HISTORY_LENGTH, 0, MAXIMUM_MS);
    if (ImPlot::BeginPlot("##FrameTimes",
                          "Total Frame Time (ms)",
                          "",
                          Vec2(-1, GRAPH_HEIGHT),
                          ImPlotFlags_None,
                          ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoDecorations,
                          ImPlotAxisFlags_Lock)) {
        ImPlot::SetLegendLocation(ImPlotLocation_North, ImPlotOrientation_Horizontal, true);
        DRAW_NOW_LINE;
        frame_time[frame] = current_frame_time;
        ImPlot::PlotLine("Raw", frame_time, HISTORY_LENGTH);

        const u32 SAMPLES_IN_AVERAGE = Math::min<u32>(30, HISTORY_LENGTH);
        u32 start_index = (HISTORY_LENGTH + frame - SAMPLES_IN_AVERAGE) % HISTORY_LENGTH;
        f32 average = 0.0;
        for (u32 i = 0; i < SAMPLES_IN_AVERAGE; i++) {
            u32 sample_index = (start_index + i) % HISTORY_LENGTH;
            average += frame_time[sample_index];
        }
        average /= SAMPLES_IN_AVERAGE;

        f32 xs[] = { -100, (f32)start_index, (f32)frame, HISTORY_LENGTH };
        f32 ys[] = { average, average, average, average };
        ImPlot::PlotLine("Avg.", xs, ys, LEN(xs));
        ImPlot::PlotScatter("Avg.", xs, ys, LEN(xs));
        ImPlot::EndPlot();
    }

    ImPlot::SetNextPlotLimits(0, HISTORY_LENGTH, 0, MAXIMUM_MS);
    if (ImPlot::BeginPlot("##TotalTimes",
                          "Total Time (ms)",
                          "",
                          Vec2(-1, GRAPH_HEIGHT),
                          ImPlotFlags_None,
                          ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoDecorations,
                          ImPlotAxisFlags_Lock)) {
        ImPlot::SetLegendLocation(ImPlotLocation_North, ImPlotOrientation_Horizontal, true);
        DRAW_NOW_LINE;

        LOCK();
        for (auto &[hash, counter] : metrics) {
            counter.total_hist[frame] = NANO_TO_MS * counter.total_nano_seconds;
            ImPlot::PlotLine(counter.name, counter.total_hist, HISTORY_LENGTH);

            counter.time_per_hist[frame] = NANO_TO_MS * counter.total_nano_seconds / (counter.num_calls ?: 1);
            ImPlot::PlotLine(counter.name, counter.time_per_hist, HISTORY_LENGTH);
        }
        UNLOCK();

        ImPlot::EndPlot();
    }

    ImPlot::SetNextPlotLimits(0, 1, 0, 1, ImGuiCond_Always);
    if (ImPlot::BeginPlot("Number of calls",
                          nullptr,
                          nullptr,
                          Vec2(GRAPH_HEIGHT * 2, GRAPH_HEIGHT),
                          ImPlotFlags_NoMousePos,
                          ImPlotAxisFlags_NoDecorations,
                          ImPlotAxisFlags_NoDecorations)) {
        ImPlot::SetLegendLocation(ImPlotLocation_NorthEast, ImPlotOrientation_Vertical, true);
        const u32 MAX_NUM_PIE_PARTS = 128;
        const char *labels[MAX_NUM_PIE_PARTS];
        u32 calls[MAX_NUM_PIE_PARTS];
        u32 i = 0;
        u32 total = 0;

        LOCK();
        for (auto &[hash, counter] : metrics) {
            if (i >= MAX_NUM_PIE_PARTS) break;
            labels[i] = counter.name;
            calls[i] = counter.num_calls;
            total += counter.num_calls;
            i++;
        }
        UNLOCK();

        ImPlot::PlotPieChart(labels, calls, i, 0.5, 0.5, 0.4, true, "%.0f");
        ImPlot::EndPlot();
        ImGui::SameLine();
        ImGui::Text("Total Number of Calls: %d", total);
    }

    LOCK();
    for (auto &[hash, counter] : metrics) {
        counter.num_calls = 0;
        counter.total_nano_seconds = 0;
    }
    UNLOCK();

    ImGui::End();
}
#else // Without IMGUI
void report() {
    LOCK();
    for (auto &[hash, counter] : metrics) {
        LOG("{} - #{} {}ns {}ns/call",
            counter.name,
            counter.num_calls,
            counter.total_nano_seconds,
            counter.total_nano_seconds / (counter.num_calls ?: 1));
        counter.num_calls = 0;
        counter.total_nano_seconds = 0;
    }
    UNLOCK();
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
} // namespace Performance
