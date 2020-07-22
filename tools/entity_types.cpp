/*
 * Do not edit this file directly! It is generated from
 * `tools/entity_types.cpp` by the build system at compile time.
 */

#include "../math/types.h"

#include "entity_types.h"

i32 format(char *buffer, u32 size, FormatHint args, EntityType type) {
    switch (type) {
$type_formats
        case EntityType::NUM_ENTITY_TYPES: break;
    }
    UNREACHABLE("Unknown entity type");
    return -1;
}

/*
 * Included from `tools/entity_types_type_of.cpp`
 */

$type_ofs

/*
 * End of `tools/entity_types_type_of.cpp`
 */
