#pragma once

#include <queue>

#include "math/smek_math.h"
#include "entity/entity.h"

namespace EventSystem {

enum EventType {
    CREATE_SOUND_ENTITY,
    CREATE_PLAYER,
    CREATE_LIGHT,

    NUM_TYPES,
};

struct EventCreateSoundEntity {
    AssetID asset_id;
    Audio::SoundSourceSettings source_settings;

    void callback();
};

struct EventCreatePlayer {
    void callback();
};

struct EventCreateLight {
    void callback();
};

struct Event {
    EventType type;
    union {
        EventCreateSoundEntity CREATE_SOUND_ENTITY;
        EventCreatePlayer CREATE_PLAYER;
        EventCreateLight CREATE_LIGHT;
    };
};

void handle_events();

}  // namespace EventSystem
