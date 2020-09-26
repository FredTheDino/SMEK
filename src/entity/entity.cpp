#include "entity.h"
#include "entity_types.h"
#include "../asset/asset.h"
#include "../renderer/renderer.h"
#include "../game.h"
#include "../test.h"
#include "imgui/imgui.h"

// These helper functions make it easier to
// create the ClassName::imgui() functions, where
// it's needed. The code will be stubbed if imgui
// and make sure each id is unique to avoid duplicate
// editing.
#ifdef IMGUI_ENABLE

#define IMPL_IMGUI(ClassName, f)     \
    void ClassName ::imgui() {       \
        ImGui::PushID(this);         \
        ImGui::Text(STR(ClassName)); \
        auto f_inst = f;             \
        f_inst();                    \
        ImGui::PopID();              \
    }

#else

#define IMPL_IMGUI(ClassName, f) \
    void ClassName ::imgui() {}

#endif

EntitySystem *entity_system() {
    return &GAMESTATE()->entity_system;
}

void Light::update() {
    if (length_squared(color) != 0.0) {
        if (light_id == NONE) {
            // Try to aquire a light ID
            for (u32 i = 0; i < GFX::MAX_LIGHTS; i++) {
                if (length_squared(GFX::lighting()->light_colors[i]) != 0)
                    continue;
                light_id = i;
                break;
            }
        }
        if (light_id == NONE) return;
        GFX::lighting()->light_colors[light_id] = color;
        GFX::lighting()->light_positions[light_id] = position;
    } else {
        if (light_id != NONE) on_remove();
    }
}

IMPL_IMGUI(Light, ([&] {
               i32 copy_id = light_id;
               ImGui::InputInt("#Light ID:", &copy_id);
               ImGui::InputFloat3("Position", position._);
               ImGui::ColorEdit3("Color", color._);
               ImGui::Checkbox("Draw As Point", &draw_as_point);
           }))

void Light::draw() {
    if (draw_as_point) {
        if (light_id == NONE) {
            GFX::push_point(position + Vec3(0.01, 0.0, 0.0), Vec4(1.0, 0.0, 0.0, 1.0), 0.07);
            GFX::push_point(position, Vec4(color.x, color.y, color.z, 0.2), 0.05);
        } else {
            GFX::push_point(position, Vec4(color.x, color.y, color.z, 1.0), 0.1);
        }
    }
}

void Light::on_remove() {
    // Black means the color isn't used.
    if (light_id != NONE) {
        GFX::lighting()->light_colors[light_id] = Vec3();
        light_id = NONE;
    }
}

void Player::update() {
    const f32 floor = 0.2;

    Vec2 turn = Input::mouse_move();
    turn = turn * delta() * GAMESTATE()->player_mouse_sensitivity;
    rotation = normalized(H::from(0.0, -turn.x, 0.0) * rotation * H::from(-turn.y, 0.0, 0.0));

    f32 drag_coef = Math::pow(0.05, delta());
    velocity.x = velocity.x * drag_coef;
    velocity.y -= 4.82 * delta(); // Temporary gravity
    velocity.z = velocity.z * drag_coef;
    velocity += rotation * Vec3(Input::value(Ac::MoveX), 0.0, Input::value(Ac::MoveZ)) * GAMESTATE()->player_movement_speed * delta();
    // Plane collision
    if (position.y <= floor) {
        position.y = floor;
        velocity.y = 0.0;

        // If grounded
        if (Input::pressed(Ac::Jump) && velocity.y == 0.0) {
            velocity.y = GAMESTATE()->player_jump_speed;
        }
    }

    if (Input::pressed(Ac::Shoot)) {
        LOG("Pew!");
    }

    if (GFX::current_camera() != GFX::debug_camera()) {
        position += velocity * delta();
    }

    GFX::gameplay_camera()->position = position + Vec3(0.0, 0.5, 0.0);
    GFX::gameplay_camera()->rotation = rotation;
}

IMPL_IMGUI(Player, ([&]() {
               ImGui::SliderFloat("Jump speed",
                                  &GAMESTATE()->player_jump_speed,
                                  0.0, 10.0,
                                  "%.2f");
               ImGui::SliderFloat("Movement speed",
                                  &GAMESTATE()->player_movement_speed,
                                  0.0, 10.0,
                                  "%.2f");
               ImGui::SliderFloat("Mouse sensitivity",
                                  &GAMESTATE()->player_mouse_sensitivity,
                                  0.0, 2.0,
                                  "%.2f");
           }))

void Player::draw() {
    scale = Vec3(1., 2., 3.) * 0.3;
    GFX::push_mesh("MONKEY", "TILES", position, rotation, scale);
}

void SoundEntity::update() {
    remove |= !GAMESTATE()->audio_struct->is_valid(audio_id);
}

void SoundEntity::imgui() {
#ifdef IMGUI_ENABLE
    Audio::SoundSource *source = GAMESTATE()->audio_struct->fetch_source(audio_id);
    if (!source) return;
    ImGui::BeginChild("Sound entities");
    ImGui::PushID(this);
    ImGui::Indent();
    ImGui::Text("SoundEntity");
    ImGui::SameLine();
    ImGui::Text("%.2f/%.2f",
                (f32)source->sample / source->sample_rate,
                (f32)source->num_samples / source->channels / source->sample_rate);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0);
    f32 gain = source->gain;
    if (ImGui::SliderFloat("Gain", &gain, 0.0, 1.0, "%.2f")) {
        GAMESTATE()->audio_struct->lock();
        source->gain = gain;
        GAMESTATE()->audio_struct->unlock();
    }
    ImGui::SameLine();
    if (ImGui::Button("Play/pause")) GAMESTATE()->audio_struct->toggle_pause_sound(audio_id);
    ImGui::SameLine();
    remove |= ImGui::Button("Stop");
    ImGui::Unindent();
    ImGui::PopID();
    ImGui::EndChild();
#endif
}

void SoundEntity::draw() {}

void SoundEntity::on_create() {
    audio_id = GAMESTATE()->audio_struct->play_sound(*this);
}

void SoundEntity::on_remove() {
    GAMESTATE()->audio_struct->stop_sound(audio_id);
}

bool EntitySystem::is_valid(EntityID id) { return entities.contains(id); }

void EntitySystem::remove(EntityID id) {
    ASSERT(is_valid(id), "Cannot remove entity that doesn't exist");
    entities[id]->on_remove();
    delete entities[id];
    entities.erase(id);
}

void EntitySystem::update() {
    for (auto [_, e] : entities) {
        e->update();
    }
    for (auto i = entities.begin(), last = entities.end(); i != last;) {
        BaseEntity *e = i->second;
        if (e->remove) {
            e->on_remove();
            delete e;
            i = entities.erase(i);
        } else {
            i++;
        }
    }
}

void EntitySystem::draw() {
#ifdef IMGUI_ENABLE
    if (GAMESTATE()->imgui.entities_enabled) {
        ImGui::Begin("Entities");
        ImGui::Text("Current entities: %ld", entities.size());

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

        for (auto [_, e] : entities) {
            e->imgui();
        }
        ImGui::End();
    }
#endif
    for (auto [_, e] : entities) {
        e->draw();
    }
}

TEST_CASE("entity_adding", {
    int calls;
    struct TestEnt : public Entity {
        int *value;
        void update() { value[0]++; };
        void draw() {}
    };

    TestEnt a;
    a.value = &calls;
    auto a_id = entity_system()->add(a);

    TestEnt b;
    b.value = &calls;
    auto b_id = entity_system()->add(b);
    calls = 0;
    entity_system()->update();
    ASSERT(calls == 2, "Got {} calls to update.", calls);

    entity_system()->remove(b_id);
    calls = 0;
    entity_system()->update();
    ASSERT(calls == 1, "Got {} calls to update.", calls);
    return true;
});

TEST_CASE("entity_remove", {
    int calls;
    struct TestEnt : public Entity {
        int *value;
        ~TestEnt() { value[0] *= 4; }
        void update() { value[0]++; };
        void draw() {}
    };

    TestEnt b;
    b.value = &calls;
    auto b_id = entity_system()->add(b);
    calls = 0;
    entity_system()->update(); // + 1
    ASSERT(calls == 1, "Got {} calls to update.", calls);

    entity_system()->remove(b_id); // * 4
    entity_system()->update();     // Should not add
    ASSERT(calls == 4, "Got {} calls to update.", calls);
    return true;
});

TEST_CASE("entity_id_valid", {
    struct TestEnt : public Entity {
        void update() {};
        void draw() {}
    };

    TestEnt b;
    auto b_id = entity_system()->add(b);
    ASSERT(entity_system()->is_valid(b_id), "Id should be valid");
    entity_system()->remove(b_id); // * 4
    ASSERT(!entity_system()->is_valid(b_id), "Id should not be valid");
    return true;
});

TEST_CASE("entity fetch", {
    struct TestEnt : public BaseEntity {
        int value = 1;
    };
    TestEnt a;
    TestEnt *a_ptr;

    ASSERT_EQ(a.value, 1);
    auto id = entity_system()->add(a);
    a_ptr = entity_system()->fetch<TestEnt>(id);
    ASSERT_EQ(a_ptr->value, 1);
    a_ptr->value = 2;
    ASSERT_EQ(a_ptr->value, 2);
    return true;
});

TEST_CASE("entity on_create", {
    struct TestEnt : public BaseEntity {
        int value = 1;
        void on_create() override {
            value = 2;
        }
    };
    TestEnt a;
    ASSERT_EQ(a.value, 1);
    auto id = entity_system()->add(a);
    TestEnt *a_ptr = entity_system()->fetch<TestEnt>(id);
    ASSERT_EQ(a_ptr->value, 2);
    return true;
});

TEST_CASE("entity on_remove", {
    struct TestEnt : public BaseEntity {
        int value = 1;
        void on_remove() override { value = 2; }
    };
    TestEnt a;
    ASSERT_EQ(a.value, 1);
    auto id = entity_system()->add(a);
    TestEnt *a_ptr = entity_system()->fetch<TestEnt>(id);
    ASSERT_EQ(a_ptr->value, 1);
    entity_system()->remove(id);
    ASSERT_EQ(a_ptr->value, 2);
    return true;
});

#undef IMPL_IMGUI
