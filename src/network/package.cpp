#include "package.h"

#include "imgui/imgui.h"
#include <cstring>

void pack(Package *package, u8 *into) {
    std::memcpy(into, package, sizeof(Package));
}

Package unpack(u8 *from) {
    Package package;
    std::memcpy(&package, from, sizeof(Package));
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
    default:
        return snprintf(buffer, size, "Unknown package type");
    }
}

#ifndef IMGUI_DISABLE
void imgui_package_create(Package *package) {
    static int type_current_id = 0;
    ImGui::Combo("", &type_current_id, package_type_list, IM_ARRAYSIZE(package_type_list));
    
    package->type = (PackageType) type_current_id;
    switch (package->type) {
    case PackageType::A:
        static int a_a = 0;
        ImGui::InputInt("a", &a_a);
        package->A.a = a_a;
        break;
    case PackageType::B:
        static int b_a = 0;
        static int b_b = 0;
        ImGui::InputInt("a", &b_a);
        ImGui::InputInt("b", &b_b);
        package->B.a = b_a;
        package->B.b = b_b;
        break;
    default:
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