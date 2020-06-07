#pragma once

#include <queue>

#include "math/smek_math.h"
#include "entity/entity.h"

namespace EventSystem {

enum EventType {
    CREATE_SOUND_ENTITY,

    NUM_TYPES,
};

struct EventCreateSoundEntity {
    //TODO(gu) Wrap in existing sound source settings etc.
    // Becomes "non-trivial" for now. std::variant?
    u64 asset_id_hash;
    f32 gain;
    bool repeat;

    void callback();
};

struct Event {
    EventType type;
    union {
        EventCreateSoundEntity CREATE_SOUND_ENTITY;
    };
};

void handle_events();

}  // namespace EventSystem
