#pragma once
#include <unordered_map>
#include "../math/smek_math.h"
#include "../math/smek_vec.h"
#include "../math/smek_quat.h"

using EntityID = u64;

// The base entity class
struct Entity {
    // This is so normal that all entities should have it.
    Vec3 position;
    Vec3 scale;
    Quat rotation;

    virtual ~Entity() {};
    virtual void update() = 0;
    virtual void draw() = 0;
};

struct Player: public Entity {
    void update() override;
    void draw() override;
};

struct EntitySystem {
    EntityID next_id;
    std::unordered_map<u64, Entity*> entities;

    bool valid(EntityID id);

    template<typename E>
    EntityID add(E entity);

    void remove(EntityID entity);

    void draw();
    void update();

    template<typename E>
    E *fetch(EntityID id);
};

template<typename E>
E *EntitySystem::fetch(EntityID id) {
    ASSERT(valid(id), "Cannot fetch entity that doesn't exist");
    return dynamic_cast<E*>(entities[id]);
}

template<typename E>
EntityID EntitySystem::add(E entity) {
    EntityID id = next_id++;
    ASSERT(!valid(id), "Adding multiple entity ids for one id");
    E *e = new E();
    *e = entity;
    entities[id] = (Entity *) e;
    return id;
}

EntitySystem *entity_system();
