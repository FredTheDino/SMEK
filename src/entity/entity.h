#pragma once
#include <unordered_map>
#include "../math/smek_math.h"
#include "../math/smek_vec.h"
#include "../math/smek_quat.h"
#include "../audio.h"

///# Entity system
//

using EntityID = u64;

struct BaseEntity {
    bool remove = false;

    virtual ~BaseEntity() {};
    virtual void update() {};
    virtual void draw() {};
    virtual void on_remove() {};
};

struct SoundEntity: public BaseEntity {
    AssetID asset_id;
    Audio::SoundSourceSettings sound_source_settings;

    AudioID audio_id;

    void update() override;
    void draw() override;
    void on_remove() override;
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

#if 0

///* BaseEntity
// The base entity all other entities inherit from.
struct BaseEntity {
    bool remove = false;
    virtual ~BaseEntity() {}
    virtual void update() {}
    virtual void draw() {}
    virtual void on_remove() {}
}
//
// <code>remove</code> is checked for all entities after each update cycle.

///* SoundEntity
// A sound entity is an entity that plays a sound (no kidding).
// Removing the entity stops the sound from playing, and,
// equivalently, the entity is removed when the sound is done playing.
struct SoundEntity: public BaseEntity {
    AssetID asset_id;
    Audio::SoundSourceSettings sound_source_settings;
    AudioID audio_id;
};

#endif
