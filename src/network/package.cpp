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
    switch (pkg.header.type) {
    case PackageType::A:
        return snprintf(buffer, size, "A: a=%0*d",
                        args.num_zero_pad, pkg.A.a);
    case PackageType::B:
        return snprintf(buffer, size, "B: a=%0*d, b=%0*d",
                        args.num_zero_pad, pkg.B.a,
                        args.num_zero_pad, pkg.B.b);
    case PackageType::SET_CLIENT_ID:
        return snprintf(buffer, size, "SetClientID: id=%0*u",
                        args.num_zero_pad, pkg.SET_CLIENT_ID.client_id);
    case PackageType::EVENT:
        return snprintf(buffer, size, "Event: type=%0*u",
                        args.num_zero_pad, (u32)pkg.EVENT.event.type);
    default:
        return snprintf(buffer, size, "Not implemented for package type %u", (u32)pkg.header.type);
    }
}

i32 format(char *buffer, u32 size, FormatHint args, PackageHeader header) {
    return snprintf(buffer, size, "client=%0*u id=%0*u",
                    args.num_zero_pad, header.client,
                    args.num_zero_pad, header.id);
}

#ifndef IMGUI_DISABLE
void imgui_package_create(Package *package, WipEntities *wip_entities) {
    int pkg_type_current_id = (int)package->header.type;
    ImGui::Combo("Package type", &pkg_type_current_id, package_type_list, LEN(package_type_list));
    package->header.type = (PackageType)pkg_type_current_id;
    switch (package->header.type) {
    case PackageType::A:
        ImGui::InputInt("a", &package->A.a);
        break;
    case PackageType::B:
        ImGui::InputInt("a", &package->B.a);
        ImGui::InputInt("b", &package->B.b);
        break;
    case PackageType::EVENT:
        imgui_event_create(&package->EVENT.event, wip_entities);
        break;
    default:
        ImGui::Text("Not implemented for package type %u", (u32)package->header.type);
        break;
    }
}

void imgui_event_create(Event *event, WipEntities *wip_entities) {
    int event_type_current_id = (int)event->type;
    ImGui::Combo("Event type", &event_type_current_id, event_type_names, LEN(event_type_names));
    event->type = (EventType)event_type_current_id;
    switch (event->type) {
    case EventType::CREATE_ENTITY: {
        int entity_type_current_id = (int)wip_entities->type;
        ImGui::Combo("Entity type", &entity_type_current_id, entity_type_names, LEN(entity_type_names));
        wip_entities->type = (EntityType)entity_type_current_id;
        switch (wip_entities->type) {
        case EntityType::LIGHT:
            wip_entities->light->imgui_create();
            *event = entity_event(*wip_entities->light);
            break;
        default:
            ImGui::Text("Not implemented for entity type %u", (u32)wip_entities->type);
            break;
        }
        break;
    }
    default:
        ImGui::Text("Not implemented for event type %u", (u32)event->type);
        break;
    }
}

void imgui_package_show(Package *package) {
    switch (package->header.type) {
    case PackageType::A:
        ImGui::Text("A\n a=%d", package->A.a);
        break;
    case PackageType::B:
        ImGui::Text("B\n a=%d\n b=%d", package->B.a, package->B.b);
        break;
    default:
        ImGui::Text("Not implemented for package type %u", (u32)package->header.type);
        break;
    }
}
#endif
