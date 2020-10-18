#include "performance.h"
#include "log.h"
#include <unordered_map>

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

void report() {
    for (auto &[hash, counter] : gpc.metrics) {
        LOG("{} - #{} {}ns {}ns/call",
            counter.name,
            counter.num_calls,
            counter.total_nano_seconds,
            counter.total_nano_seconds / (counter.num_calls != 0 ?: 1));
        counter.num_calls = 0;
        counter.total_nano_seconds = 0;
    }
}
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
