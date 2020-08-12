#pragma once

#include "../util/util.h"
#include "../math/types.h"
#include "../event.h"

enum class PackageType {
    A,
    B,
    EVENT,

    // Types below this line should not be creatable and thus not shown in the package type list.

    SET_CLIENT_ID,

    NUM_TYPES,
};

static const char *package_type_list[] = {
    "A",
    "B",
    "Create event",
};

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
    u32 id;
};

struct Package {
    u32 client;
    u32 id;
    PackageType type;
    union {
        PackageA A;
        PackageB B;
        PackageEvent EVENT;
        PackageSetClientID SET_CLIENT_ID;
    };
};

struct WipEntities {
    EntityType type;
    Light *light;

    void alloc();
    void free();
};

void pack(u8 *into, Package *from);
void unpack(Package *into, u8 *from);
Package unpack(u8 *from);

i32 format(char *buffer, u32 size, FormatHint args, Package pkg);

#ifndef IMGUI_DISABLE
void imgui_package_show(Package *package);
void imgui_entity_create(Light *light);
void imgui_package_create(Package *package, WipEntities *wip_entities);
void imgui_event_create(Package *package, WipEntities *wip_entities);
#endif
