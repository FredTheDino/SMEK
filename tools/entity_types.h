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
$types

    NUM_ENTITY_TYPES,
};

static const char *entity_type_names[] = {
$type_names
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

$type_ofs
/*
 * End of `tools/entity_types_type_of.h`
 */

struct EventCreateEntity {
    bool generate_id;
    EntityType type;
    union {
$event_entity_bytes_union
    };

    void callback();
};

struct Event;

$entity_events_prototypes
