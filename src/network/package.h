#pragma once

#include "../util/util.h"
#include "../math/types.h"
#include "../event.h"

///# Packages
// Packages are a way of standardizing the content of what the instances send
// to each other.

///*
enum class PackageType {
    A,
    B,
    EVENT,
    HEARTBEAT,

    // Types below this line should not be creatable and thus not shown in the package type list.
    _NUM_NAMED_TYPES,

    SET_CLIENT_ID,

    _NUM_TYPES,
};

///*
// This has to match the order in the above package type enum.
// Types not represented by a string here won't be creatable,
// which is why creatable packages should be grouped at the top above.
static const char *package_type_list[] = {
    "A",
    "B",
    "Create event",
    "Heartbeat",
};

static_assert(!(LEN(package_type_list) < (u64)PackageType::_NUM_NAMED_TYPES), "Too few package type names");
static_assert(!(LEN(package_type_list) > (u64)PackageType::_NUM_NAMED_TYPES), "Too many package type names");

struct PackageA {
    int a;
};

struct PackageB {
    int b;
    int a;
};

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
        PackageA A;
        PackageB B;
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

///*
// Like above, but returns an actual value.
Package unpack(u8 *from);

i32 format(char *buffer, u32 size, FormatHint args, Package pkg);
i32 format(char *buffer, u32 size, FormatHint args, PackageHeader header);

#ifndef IMGUI_DISABLE
void imgui_package_create(Package *package, WipEntities *wip_entities);
void imgui_event_create(Event *event, WipEntities *wip_entities);
void imgui_package_show(Package *package);
#endif
