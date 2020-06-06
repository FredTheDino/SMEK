#include "entity.h"
#include "../asset/asset.h"
#include "../renderer/renderer.h"
#include "../game.h"
#include "../test.h"

EntitySystem *entity_system() {
    return &GAMESTATE()->entity_system;
}

void Player::update() {
    position = Vec3(Math::cos(time()) * 0.2, Math::sin(time()) * 0.2, -0.5);
}

void Player::draw() {
    GFX::MasterShader shader = GFX::master_shader();
    GFX::Mesh mesh = *Asset::fetch_mesh("MONKEY");
    Mat model_matrix = Mat::translate(position) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();
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
}

void EntitySystem::draw() {
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
    ASSERT(calls == 2, "Got %d calls to update.", calls);

    entity_system()->remove(b_id);
    calls = 0;
    entity_system()->update();
    ASSERT(calls == 1, "Got %d calls to update.", calls);
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
