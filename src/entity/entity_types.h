/*
 * Do not edit this file directly! It is generated from
 * `tools/entity_types.h` by the build system at compile time.
 */

#pragma once
#include <typeinfo>

#include "entity.h"
#include "../util/util.h"

#include "../asset/asset.h"
#include "../audio.h"
#include "../math/types.h"

enum class EntityType {
    BASEENTITY,
    ENTITY,
    LIGHT,
    PLAYER,
    SOUNDENTITY,

    NUM_ENTITY_TYPES,
};

i32 format(char *, u32, FormatHint, EntityType);

struct Field {
    const std::type_info &typeinfo;
    const char *name;
    int size;
    int offset;
};

struct FieldList {
    int num_fields;
    Field *list;
};

///*
// Returns a list of fields on the specified struct type.
FieldList get_fields_for(EntityType type);

/*
 * Included from `tools/entity_types_type_of.h`
 */

struct BaseEntity;
EntityType type_of(BaseEntity *);

struct Entity;
EntityType type_of(Entity *);

struct Light;
EntityType type_of(Light *);

struct Player;
EntityType type_of(Player *);

struct SoundEntity;
EntityType type_of(SoundEntity *);

/*
 * End of `tools/entity_types_type_of.h`
 */

struct EventCreateEntity {
    EntityType type;
    union {
        u8 BASEENTITY[sizeof(BaseEntity) - sizeof(void *)];
        u8 ENTITY[sizeof(Entity) - sizeof(void *)];
        u8 LIGHT[sizeof(Light) - sizeof(void *)];
        u8 PLAYER[sizeof(Player) - sizeof(void *)];
        u8 SOUNDENTITY[sizeof(SoundEntity) - sizeof(void *)];
    };

    void callback();
};

namespace EventSystem {
struct Event;
}

EventSystem::Event entity_event(BaseEntity);
EventSystem::Event entity_event(Entity);
EventSystem::Event entity_event(Light);
EventSystem::Event entity_event(Player);
EventSystem::Event entity_event(SoundEntity);
