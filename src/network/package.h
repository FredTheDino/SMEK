#pragma once

#include "../util/util.h"
#include "../math/types.h"
#include "../event.h"

///# Packages

///*
enum class PackageType {
    EVENT,
    HEARTBEAT,

    // Types below this line should not be creatable and thus not shown in the package type list.
    _NUM_NAMED_TYPES,

    SET_CLIENT_ID,

    _NUM_TYPES,
};

// This has to match the order in the above package type enum.
// Types not represented by a string here won't be creatable,
// which is why creatable packages should be grouped at the top above.
///*
static const char *package_type_list[] = {
    "Create event",
    "Heartbeat",
};

static_assert(!(LEN(package_type_list) < (u64)PackageType::_NUM_NAMED_TYPES), "Too few package type names");
static_assert(!(LEN(package_type_list) > (u64)PackageType::_NUM_NAMED_TYPES), "Too many package type names");

struct PackageEvent {
    Event event;
};

struct PackageSetClientID {
    u32 client_id;
};

struct PackageHeartbeat {
    u32 id;
};

///*
struct PackageHeader {
    u32 client;
    u32 id;
    PackageType type;
};

///*
struct Package {
    PackageHeader header;
    union {
        PackageEvent EVENT;
        PackageSetClientID SET_CLIENT_ID;
        PackageHeartbeat HEARTBEAT;
    };
};

struct WipEntities {
    EntityType type;
    Light *light;

    void alloc();
    void free();
};

///*
void pack(u8 *into, Package *from);

///*
void unpack(Package *into, u8 *from);
Package unpack(u8 *from);

i32 format(char *buffer, u32 size, FormatHint args, Package pkg);
i32 format(char *buffer, u32 size, FormatHint args, PackageHeader header);

#ifdef IMGUI_ACTIVE
void imgui_package_create(Package *package, WipEntities *wip_entities);
void imgui_event_create(Event *event, WipEntities *wip_entities);
void imgui_package_show(Package *package);
#endif
