#pragma once

#include <queue>

#include "math/smek_math.h"
#include "entity/entity.h"
#include "entity/entity_types.h"

enum EventType {
    CREATE_ENTITY,

    NUM_TYPES,
};

static const char *event_type_names[] = {
    "CreateEntity",
};

struct Event {
    EventType type;
    union {
        EventCreateEntity CREATE_ENTITY;
    };
};

void handle_events();
