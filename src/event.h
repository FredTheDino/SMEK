#pragma once

#include <queue>

#include "math/smek_math.h"
#include "entity/entity.h"
#include "entity/entity_types.h"
#include "network/events.h"

enum EventType {
    CREATE_ENTITY,
    LIGHT_UPDATE,
    PLAYER_INPUT,
    PLAYER_UPDATE,
    DROP_CLIENT,

    _NUM_TYPES,
};

static const char *event_type_names[] = {
    "CreateEntity",
    "Update light",
    "Player input",
    "Player update",
    "Drop client",
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
        DropClientEvent DROP_CLIENT;
    };
};

void handle_events();
