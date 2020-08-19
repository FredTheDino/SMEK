#pragma once

#include <queue>

#include "math/smek_math.h"
#include "entity/entity.h"
#include "entity/entity_types.h"

namespace EventSystem {

enum EventType {
    CREATE_ENTITY,

    NUM_TYPES,
};

struct Event {
    EventType type;
    union {
        EventCreateEntity CREATE_ENTITY;
    };
};

void handle_events();

} // namespace EventSystem
