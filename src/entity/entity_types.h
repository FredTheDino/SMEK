/*
 * Do not edit this file directly! It is generated from
 * `tools/entity_types.h` by the build system at compile time.
 */

#pragma once
#include <typeinfo>

#include "../util/util.h"

enum class EntityType {
    BASEENTITY,
    ENTITY,
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

struct Player;
EntityType type_of(Player *);

struct SoundEntity;
EntityType type_of(SoundEntity *);


/*
 * End of `tools/entity_types_type_of.h`
 */
