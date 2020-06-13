#include "entity.h"
#include "../asset/asset.h"
#include "../renderer/renderer.h"
#include "../game.h"
#include "../test.h"
#include "imgui/imgui.h"

EntitySystem *entity_system() {
    return &GAMESTATE()->entity_system;
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
    position += velocity * delta();

    GFX::main_camera()->position = position + Vec3(0.0, 0.3, 0.0);
    GFX::main_camera()->rotation = rotation;
}

void Player::draw() {
#ifndef IMGUI_DISABLE
    ImGui::BeginChild("Player");
    ImGui::SliderFloat("Jump speed", &GAMESTATE()->player_jump_speed, 0.0, 10.0, "%.2f");
    ImGui::SliderFloat("Movement speed", &GAMESTATE()->player_movement_speed, 0.0, 10.0, "%.2f");
    ImGui::SliderFloat("Mouse sensitivity", &GAMESTATE()->player_mouse_sensitivity, 0.0, 2.0, "%.2f");
    ImGui::EndChild();
#endif

    GFX::MasterShader shader = GFX::master_shader();
    GFX::Mesh mesh = *Asset::fetch_mesh("MONKEY");
    Mat model_matrix = Mat::translate(position) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();
}

void SoundEntity::update() {
    remove |= !GAMESTATE()->audio_struct->is_valid(audio_id);
}

void SoundEntity::draw() {
#ifndef IMGUI_DISABLE
    Audio::SoundSource *source = GAMESTATE()->audio_struct->fetch_source(audio_id);
    if (!source) return;
    ImGui::BeginChild("Sound entities");
    ImGui::PushID(this);
    ImGui::Indent();
    ImGui::Text(asset_id.name ? asset_id.name : "NO_NAME");
    ImGui::SameLine();
    ImGui::Text("%.2f/%.2f",
                (f32) source->sample / source->sample_rate,
                (f32) source->num_samples / source->channels / source->sample_rate);
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

void SoundEntity::on_remove() {
    GAMESTATE()->audio_struct->stop_sound(audio_id);
}

bool EntitySystem::valid(EntityID id) {
    return entities.count(id);
}

void EntitySystem::remove(EntityID id) {
    ASSERT(valid(id), "Cannot remove entity that doesn't exist");
    entities[id]->on_remove();
    delete entities[id];
    entities.erase(id);
}

void EntitySystem::update() {
    for (auto [_, e]: entities) {
        e->update();
    }
    for (auto i = entities.begin(), last = entities.end(); i != last; ) {
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
#ifndef IMGUI_DISABLE
    ImGui::Begin("Entities");
    ImGui::Text("Current entities: %ld", entities.size());

    ImGui::BeginChild("Sound entities", ImVec2(0, 128), true);
    ImGui::Text("Sound entities:");
    if (ImGui::Button("Create sound"))
        GAMESTATE()->show_create_sound_window = true;
    ImGui::SameLine();
    if (ImGui::Button("Stop all sounds"))
        GAMESTATE()->audio_struct->stop_all();
    ImGui::Spacing();
    ImGui::EndChild();

    ImGui::BeginChild("Player", ImVec2(0, 100), true);
    ImGui::Text("Player:");
    ImGui::EndChild();
#endif
    for (auto [_, e]: entities) {
        e->draw();
    }
#ifndef IMGUI_DISABLE
    ImGui::End();
#endif
}

TEST_CASE("entity_adding", {
    int calls;
    struct TestEnt: public Entity {
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
    struct TestEnt: public Entity {
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
    entity_system()->update();  // Should not add
    ASSERT(calls == 4, "Got {} calls to update.", calls);
    return true;
});

TEST_CASE("entity_id_valid", {
    struct TestEnt: public Entity {
        void update() {};
        void draw() {}
    };

    TestEnt b;
    auto b_id = entity_system()->add(b);
    ASSERT(entity_system()->valid(b_id), "Id should be valid");
    entity_system()->remove(b_id); // * 4
    ASSERT(!entity_system()->valid(b_id), "Id should not be valid");
    return true;
});
