/*
 * Do not edit this file directly! It is generated from
 * `tools/entity_types.h` by the build system at compile time.
 */

#pragma once

#include "../util/util.h"

enum class EntityType {
    BASEENTITY,
    ENTITY,
    PLAYER,
    SOUNDENTITY,
    TESTENT,

    NUM_ENTITY_TYPES,
};

i32 format(char *, u32, FormatHint, EntityType);

/*
 * Included from `tools/entity_types_type_of.h`
 */

struct BaseEntity;
EntityType type_of(BaseEntity *);

struct Entity;
EntityType type_of(Entity *);

struct Player;
EntityType type_of(Player *);

struct SoundEntity;
EntityType type_of(SoundEntity *);

struct TestEnt;
EntityType type_of(TestEnt *);


/*
 * End of `tools/entity_types_type_of.h`
 */