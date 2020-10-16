#pragma once

#include <queue>

#include "math/smek_math.h"
#include "entity/entity.h"
#include "entity/entity_types.h"

enum EventType {
    CREATE_ENTITY,
    LIGHT_UPDATE,
    PLAYER_INPUT,
    PLAYER_UPDATE,

    _NUM_TYPES,
};

static const char *event_type_names[] = {
    "CreateEntity",
    "Update light",
    "Player input",
    "Player update",
};

static_assert(!(LEN(event_type_names) < (u64)EventType::_NUM_TYPES), "Too few event type names");
static_assert(!(LEN(event_type_names) > (u64)EventType::_NUM_TYPES), "Too many event type names");

struct Event {
    EventType type;
    union {
        EventCreateEntity CREATE_ENTITY;
        LightUpdate LIGHT_UPDATE;
        PlayerInput PLAYER_INPUT;
        PlayerUpdate PLAYER_UPDATE;
    };
};

void handle_events();
