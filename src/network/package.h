#include "../util/util.h"
#include "../math/types.h"

enum class PackageType {
    A,
    B,

    NUM_TYPES,
};

static const char *package_type_list[] = {
    "A",
    "B",
};

struct PackageA {
    int a;
};

struct PackageB {
    int b;
    int a;
};

struct Package {
    PackageType type;
    union {
        PackageA A;
        PackageB B;
    };
};

void pack(Package *package, u8 *into);
Package unpack(u8 *from);

i32 format(char *buffer, u32 size, FormatHint args, Package pkg);

#ifndef IMGUI_DISABLE
void imgui_package_show(Package *package);
void imgui_package_create(Package *package);
#endif
