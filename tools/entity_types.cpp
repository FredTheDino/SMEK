// clang-format off
/*
 * Do not edit this file directly! It is generated from
 * `tools/entity_types.cpp` by the build system at compile time.
 */
#include "entity_types.h"
#include "entity.h"

#include <cstring>
#include <stddef.h>
#include "../event.h"
#include "../game.h"

i32 format(char *buffer, u32 size, FormatHint args, EntityType type) {
    switch (type) {
$type_formats
    default: return snprintf(buffer, size, "?ENITY TYPE?");
    }
    return 0;
}

namespace FieldName {
$all_field_names_impl
};

$fields_data

FieldList get_fields_for(EntityType type) {
    switch (type) {
$fields_switch
    default:
        UNREACHABLE("Unknown entity type");
        return {};
    }
}

void emplace_entity(void *buffer, EntityType type) {
    switch (type) {
$emplace_switch
    default:
        UNREACHABLE("Unknown entity type");
    }
}

EntityID EntitySystem::add_unknown_type(BaseEntity *e) {
    switch (e->type) {
$add_switch
    default:
        UNREACHABLE("Unknown entity type");
        return {};
    }
}

/*
 * Included from `tools/entity_types_type_of.cpp`
 */

$type_ofs
/*
 * End of `tools/entity_types_type_of.cpp`
 */

/*
 * Included from `tools/entity_types_event_callback.cpp`
 */

void EventCreateEntity::callback() {
    switch (type) {
$callbacks
    default:
        UNREACHABLE("Unknown entity type");
        break;
    }
}

/*
 * End of `tools/entity_types_event_callback.cpp`
 */

/*
 * Included from `tools/entity_types_event.cpp`
 */

$entity_events
/*
 * End of `tools/entity_types_event.cpp`
 */
