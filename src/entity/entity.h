#pragma once
#include <unordered_map>
#include "../math/smek_math.h"
#include "../math/smek_vec.h"
#include "../math/smek_quat.h"
#include "../audio.h"

///# Entity system
//

using EntityID = u64;

enum class EntityType;

// TODO(ed): Rename this to
// "EntityInterface", BaseEntity makes you
// think it has fields.
struct BaseEntity {
    bool remove = false;

    virtual ~BaseEntity() {};
    virtual void update() {};
    virtual void draw() {};
    virtual void on_create() {};
    virtual void on_remove() {};

    EntityType type;
};

struct SoundEntity: public BaseEntity {
    AssetID asset_id;
    Audio::SoundSourceSettings sound_source_settings;

    AudioID audio_id;

    void update() override;
    void draw() override;
    void on_create() override;
    void on_remove() override;
};

struct Entity: public BaseEntity {
    Vec3 position;
    Vec3 scale;
    Quat rotation;
};

// NOTE(ed): It might be more practical if
// "Light" is an "Entity"
struct Light: public BaseEntity {
    static const int NONE = -1;

    i32 light_id = NONE;

    Vec3 position;
    Vec3 color;

    void update() override;
    void draw() override;
    void on_remove() override;
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
    e->type = type_of(e);
    entities[id] = (BaseEntity *) e;
    e->on_create();
    return id;
}

EntitySystem *entity_system();

/*
//// BaseEntity
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

//// SoundEntity
// A sound entity is an entity that plays a sound (no kidding).
// Removing the entity stops the sound from playing, and,
// equivalently, the entity is removed when the sound is done playing.
struct SoundEntity: public BaseEntity {
    AssetID asset_id;
    Audio::SoundSourceSettings sound_source_settings;
    AudioID audio_id;
};
*/
