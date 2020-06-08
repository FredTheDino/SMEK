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
    ImGui::SliderFloat("Jump speed", &GAMESTATE()->player_jump_speed, 0.0, 10.0, "%.2f");
    ImGui::SliderFloat("Movement speed", &GAMESTATE()->player_movement_speed, 0.0, 10.0, "%.2f");
    ImGui::SliderFloat("Mouse sensitivity", &GAMESTATE()->player_mouse_sensitivity, 0.0, 2.0, "%.2f");
    ImGui::Separator();
#endif

    GFX::MasterShader shader = GFX::master_shader();
    GFX::Mesh mesh = *Asset::fetch_mesh("MONKEY");
    Mat model_matrix = Mat::translate(position) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();
}

void SoundEntity::update() {
    remove = !GAMESTATE()->audio_struct->is_playing(audio_id);
}

bool EntitySystem::valid(EntityID id) {
    return entities.count(id);
}

void EntitySystem::remove(EntityID id) {
    ASSERT(valid(id), "Cannot remove entity that doesn't exist");
    delete entities[id];
    entities.erase(id);
}

void EntitySystem::update() {
    for (auto [_, e]: entities) {
        e->update();
    }
    std::erase_if(entities, [](const auto &item) {
        auto const &[_, e] = item;
        return e->remove;
    });
}

void EntitySystem::draw() {
#ifndef IMGUI_DISABLE
    ImGui::Text("Entities: %ld", entities.size());
    ImGui::Separator();
#endif
    for (auto [_, e]: entities) {
        e->draw();
    }
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
    ASSERT(calls == 1, "Got %d calls to update.", calls);

    entity_system()->remove(b_id); // * 4
    entity_system()->update();  // Should not add
    ASSERT(calls == 4, "Got %d calls to update.", calls);
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
