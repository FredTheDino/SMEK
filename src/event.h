#pragma once

#include <queue>

#include "math/smek_math.h"
#include "entity/entity.h"

namespace EventSystem {

enum EventType {
    CREATE_SOUND_ENTITY,
    CREATE_PLAYER,

    NUM_TYPES,
};

struct EventCreateSoundEntity {
    //TODO(gu) Wrap in existing sound source settings etc.
    // Becomes "non-trivial" for now. std::variant?
    AssetID asset_id;
    Audio::SoundSourceSettings source_settings;

    void callback();
};

struct EventCreatePlayer {
    void callback();
};

struct Event {
    EventType type;
    union {
        EventCreateSoundEntity CREATE_SOUND_ENTITY;
        EventCreatePlayer CREATE_PLAYER;
    };
};

void handle_events();

}  // namespace EventSystem
