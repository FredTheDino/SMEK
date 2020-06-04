#include "entity.h"
#include "../asset/asset.h"
#include "../renderer/renderer.h"
#include "../game.h"

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
