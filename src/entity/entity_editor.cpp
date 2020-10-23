#include "entity.h"
#include "entity_types.h"
#include "imgui/imgui.h"
#include "../game.h"
#include "../test.h"

#include <functional>
#include <cxxabi.h>

#ifdef IMGUI_ENABLE
using ImGuiDisplayFunc = std::function<void(const char *, void *)>;
static std::unordered_map<std::size_t, ImGuiDisplayFunc> func_map;

namespace ImGuiFuncs {
void empty_f(const char *name, void *) {}

void show_bool(const char *name, void *v) {
    ImGui::Checkbox(name, (bool *)v);
}

void show_Color3(const char *name, void *v) {
    ImGui::ColorEdit3(name, (f32 *)v);
}

void show_Vec3(const char *name, void *v) {
    ImGui::InputFloat3(name, (f32 *)v);
}

void show_EntityType(const char *name, void *v) {
    EntityType *e = (EntityType *)v;
    char buffer[20];
    sntprint(buffer, LEN(buffer), "{}", *e);
    ImGui::Text("Type: %s", buffer);
}

void show_u64(const char *name, void *v) {
    ImGui::InputScalar(name, ImGuiDataType_U64, v);
}

void show_u32(const char *name, void *v) {
    ImGui::InputScalar(name, ImGuiDataType_U32, v);
}

void show_u16(const char *name, void *v) {
    ImGui::InputScalar(name, ImGuiDataType_U16, v);
}

void show_u8(const char *name, void *v) {
    ImGui::InputScalar(name, ImGuiDataType_U8, v);
}

void show_i64(const char *name, void *v) {
    ImGui::InputScalar(name, ImGuiDataType_S64, v);
}

void show_i32(const char *name, void *v) {
    ImGui::InputScalar(name, ImGuiDataType_S32, v);
}

void show_i16(const char *name, void *v) {
    ImGui::InputScalar(name, ImGuiDataType_S16, v);
}

void show_i8(const char *name, void *v) {
    ImGui::InputScalar(name, ImGuiDataType_S8, v);
}

void show_H(const char *name, void *v) {
    H *quat = (H *)v;
    Vec3 euler = to_euler(*quat);
    if (ImGui::InputFloat3(name, euler._)) {
        *quat = H::from(euler.x, euler.y, euler.z);
    }
}
}

void EntitySystem::draw_imgui() {
    if (GAMESTATE()->imgui.entities_enabled) {
        ImGui::Begin("Entities");
        ImGui::Text("Current number of entities: %ld", entities.size());
        static bool initalize_func_map = true;
        if (initalize_func_map) {
#define F(T) func_map[typeid(T).hash_code()] = ImGuiFuncs::show_##T
            F(Color3);
            F(Vec3);
            F(bool);
            F(EntityType);
            F(i64);
            F(i32);
            F(i16);
            F(i8);
            F(u64);
            F(u32);
            F(u16);
            F(u8);
            F(H);
            initalize_func_map = false;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
        if (ImGui::TreeNode("Raw Entity Data")) {
            // Imgui for each entity
            for (auto [id, e] : entities) {
                //if (!selected.contains(id)) continue;

                if (!ImGui::TreeNode(e, "%lu - %s", e->entity_id, type_name(e))) continue;
                FieldList fields = get_fields_for(e->type);

                for (int i = 0; i < fields.num_fields; i++) {
                    Field f = fields.list[i];
                    std::size_t hash = f.typeinfo.hash_code();
                    if (func_map.contains(hash)) {
                        void *data = (void *)((u8 *)e + f.offset);
                        func_map[hash](f.name, data);
                    } else {
                        const char *name = f.typeinfo.name();
                        int status;
                        char *demangled = abi::__cxa_demangle(name, 0, 0, &status);
                        ASSERT_EQ(status, 0);
                        LOG("Failed to 'ImGui show' field '{}' with unsupported type '{}'", f.name, demangled);
                        func_map[hash] = ImGuiFuncs::empty_f;
                        delete[] demangled;
                    }
                    // Do stuff for all the fields,
                    // like drawing them, map of type to functions.
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        } else {
            //TODO(gu) filter for sound entities
            //TODO(gu) formatting, spacing, the whole lot
            ImGui::BeginChild("Sound entities", ImVec2(0, 170 + ((ImGui::GetTextLineHeightWithSpacing() + 6) * 4)), true);
            ImGui::Text("Sound entities:");
            if (ImGui::Button("Create sound"))
                GAMESTATE()->imgui.show_create_sound_window = true;
            ImGui::SameLine();
            if (ImGui::Button("Stop all sounds"))
                GAMESTATE()->audio_struct->stop_all();
            ImGui::Spacing();

            if (GAMESTATE()->imgui.show_create_sound_window) {
                ImGui::BeginChild("Create sound entity", ImVec2(0, 110), true);
                static AssetID item_current_idx;
                static f32 gain = 0.3;
                static bool repeat = true;

                if (!Asset::is_valid(item_current_idx)) {
                    item_current_idx = AssetID::NONE();
                }

                const char *id_preview = (Asset::is_valid(item_current_idx) ? GAMESTATE()->asset_system.assets[item_current_idx].header->name : "Sound asset");

                if (ImGui::BeginCombo("", id_preview)) {
                    for (auto &it : GAMESTATE()->asset_system.assets) {
                        Asset::UsableAsset asset = it.second;
                        if (asset.header->type != Asset::AssetType::SOUND) continue;
                        const bool is_selected = (item_current_idx == it.first);
                        if (ImGui::Selectable(asset.header->name, is_selected))
                            item_current_idx = it.first;
                    }
                    ImGui::EndCombo();
                }
                ImGui::SliderFloat("Gain", &gain, 0.0, 1.0, "%.2f");
                ImGui::Checkbox("Repeat", &repeat);

                if (ImGui::Button("Play") && (item_current_idx != AssetID::NONE())) {
                    AssetID asset_id = AssetID(GAMESTATE()->asset_system.assets[item_current_idx].header->name);
                    Asset::fetch_sound(asset_id);
                    SoundEntity sound_entity = {};
                    sound_entity.asset_id = asset_id;
                    sound_entity.sound_source_settings.gain = gain;
                    sound_entity.sound_source_settings.repeat = repeat;
                    GAMESTATE()->event_queue.push(entity_event(sound_entity));
                }
                ImGui::SameLine();
                if (ImGui::Button("Close")) GAMESTATE()->imgui.show_create_sound_window = false;
                ImGui::EndChild();
            }
            ImGui::EndChild();

            // Imgui for each entity
            for (auto [_, e] : entities) {
                e->imgui();
            }
        }
        ImGui::PopStyleVar();
        ImGui::End();
    }
}
#else
void EntitySystem::draw_imgui() {}
#endif
