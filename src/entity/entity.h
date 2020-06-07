#pragma once
#include <unordered_map>
#include "../math/smek_math.h"
#include "../math/smek_vec.h"
#include "../math/smek_quat.h"

using EntityID = u64;

struct BaseEntity {
    virtual ~BaseEntity() {};
    virtual void update() = 0;
    virtual void draw() = 0;
};

struct Entity: public BaseEntity {
    Vec3 position;
    Vec3 scale;
    Quat rotation;
};

struct Player: public Entity {
    Vec3 velocity;

    void update() override;
    void draw() override;
};

struct EntitySystem {
    EntityID next_id;
    std::unordered_map<u64, BaseEntity*> entities;

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
