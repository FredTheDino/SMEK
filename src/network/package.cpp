#include "package.h"

#include "imgui/imgui.h"
#include <cstring>

void pack(u8 *into, Package *from) {
    std::memcpy(into, from, sizeof(Package));
}

void unpack(Package *into, u8 *from) {
    std::memcpy(into, from, sizeof(Package));
}

Package unpack(u8 *from) {
    Package package;
    unpack(&package, from);
    return package;
}

i32 format(char *buffer, u32 size, FormatHint args, Package pkg) {
    switch (pkg.type) {
    case PackageType::A:
        return snprintf(buffer, size, "A: a=%0*d",
                        args.num_zero_pad, pkg.A.a);
    case PackageType::B:
        return snprintf(buffer, size, "B: a=%0*d, b=%0*d",
                        args.num_zero_pad, pkg.B.a,
                        args.num_zero_pad, pkg.B.b);
    case PackageType::SET_CLIENT_ID:
        return snprintf(buffer, size, "SetClientID: id=%0*u",
                        args.num_zero_pad, pkg.SET_CLIENT_ID.id);
    default:
        return snprintf(buffer, size, "Unknown package type");
    }
}

#ifndef IMGUI_DISABLE
void imgui_package_create(Package *package) {
    int type_current_id = (u32) package->type;
    ImGui::Combo("", &type_current_id, package_type_list, IM_ARRAYSIZE(package_type_list));
    
    package->type = (PackageType) type_current_id;
    switch (package->type) {
    case PackageType::A:
        ImGui::InputInt("a", &package->A.a);
        break;
    case PackageType::B:
        ImGui::InputInt("a", &package->B.a);
        ImGui::InputInt("b", &package->B.b);
        break;
    default:
        ImGui::Text("Unknown package type %u", (u32) package->type);
        break;
    }
}

void imgui_package_show(Package *package) {
    switch (package->type) {
    case PackageType::A:
        ImGui::Text("A\n a=%d", package->A.a);
        break;
    case PackageType::B:
        ImGui::Text("B\n a=%d\n b=%d", package->B.a, package->B.b);
        break;
    default:
        ImGui::Text("Unknown package type %u", (u32) package->type);
        break;
    }
}
#endif
