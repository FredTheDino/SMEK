#pragma once

#include <queue>

#include "math/smek_math.h"

namespace EventSystem {

enum EventType {
    A,

    NUM_TYPES,
};

struct EventA {
    f32 a;

    void callback();
};

struct Event {
    EventType type;
    union {
        EventA A;
    };
};

void handle_events();

}  // namespace EventSystem
