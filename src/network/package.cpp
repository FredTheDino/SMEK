#include "package.h"

#include "imgui/imgui.h"
#include <cstring>

void WipEntities::alloc() {
    light = new Light;
}

void WipEntities::free() {
    delete light;
}

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
    case PackageType::EVENT:
        return snprintf(buffer, size, "Event: type=%0*u",
                        args.num_zero_pad, (u32) pkg.EVENT.event.type);
    default:
        return snprintf(buffer, size, "Not implemented for package type %u", (u32) pkg.type);
    }
}

#ifndef IMGUI_DISABLE
void imgui_package_create(Package *package, WipEntities *wip_entities) {
    int pkg_type_current_id = (int) package->type;
    ImGui::Combo("Package type", &pkg_type_current_id, package_type_list, IM_ARRAYSIZE(package_type_list));
    package->type = (PackageType) pkg_type_current_id;
    switch (package->type) {
    case PackageType::A:
        ImGui::InputInt("a", &package->A.a);
        break;
    case PackageType::B:
        ImGui::InputInt("a", &package->B.a);
        ImGui::InputInt("b", &package->B.b);
        break;
    case PackageType::EVENT:
        imgui_event_create(package, wip_entities);
        break;
    default:
        ImGui::Text("Not implemented for package type %u", (u32) package->type);
        break;
    }
}

void imgui_event_create(Package *package, WipEntities *wip_entities) {
    int event_type_current_id = (int) package->EVENT.event.type;
    ImGui::Combo("Event type", &event_type_current_id, event_type_names, IM_ARRAYSIZE(event_type_names));
    package->EVENT.event.type = (EventType) event_type_current_id;
    switch (package->EVENT.event.type) {
    case EventType::CREATE_ENTITY: {
        int entity_type_current_id = (int) wip_entities->type;
        ImGui::Combo("Entity type", &entity_type_current_id, entity_type_names, IM_ARRAYSIZE(entity_type_names));
        wip_entities->type = (EntityType) entity_type_current_id;
        switch (wip_entities->type) {
        case EntityType::LIGHT:
            wip_entities->light->imgui_create();
            package->EVENT.event = entity_event(*wip_entities->light);  // possible performance sink
            break;
        default:
            ImGui::Text("Not implemented for entity type %u", (u32) wip_entities->type);
            break;
        }
        break;
    }  // case CREATE_ENTITY
    default:
        ImGui::Text("Not implemented for event type %u",  (u32) package->EVENT.event.type);
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
        ImGui::Text("Not implemented for package type %u", (u32) package->type);
        break;
    }
}
#endif
