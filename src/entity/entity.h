#pragma once
#include <unordered_map>
#include "../math/smek_math.h"
#include "../math/smek_vec.h"
#include "../math/smek_quat.h"
#include "../audio.h"

///# Entity system
//

using EntityID = u64;

///* EntityType
// A magical enum that is automatically generated by the
// build script. Parses the source code to generate this.
enum class EntityType;

///* BaseEntity
// The base entity all other entities inherit from.
struct BaseEntity {
    bool remove = false;

    virtual void imgui() {};

    virtual ~BaseEntity() {};
    virtual void update() {};
    virtual void draw() {};
    virtual void on_create() {};
    virtual void on_remove() {};

    EntityType type;
};

///* SoundEntity
// A sound entity is an entity that plays a sound (no kidding).
// Removing the entity stops the sound from playing, and,
// equivalently, the entity is removed when the sound is done playing.
struct SoundEntity : public BaseEntity {
    AssetID asset_id;
    Audio::SoundSourceSettings sound_source_settings;

    AudioID audio_id;

    void imgui() override;

    void update() override;
    void draw() override;
    void on_create() override;
    void on_remove() override;
};

///* Entity
// Adds position, scale and rotation to entities
// that derive from it.
struct Entity : public BaseEntity {
    Vec3 position;
    Vec3 scale;
    Quat rotation;
};

///* Light
// A light source in the world. Interfaces with the
// renderer under the hood and abstracts away the interface.
//
// NOTE(ed): It might be more practical if
// "Light" is an "Entity"
struct Light : public BaseEntity {
    static const int NONE = -1;

    i32 light_id = NONE;

    Vec3 position;
    Vec3 color;

    bool draw_as_point;

    void imgui() override;

    void update() override;
    void draw() override;
    void on_remove() override;
};

///* Player
// A playable character
struct Player : public Entity {
    Vec3 velocity;

    void imgui() override;

    void update() override;
    void draw() override;
};

///* EntitySystem
// The core
struct EntitySystem {
    EntityID next_id;
    std::unordered_map<EntityID, BaseEntity *> entities;

    bool is_valid(EntityID id);

    template <typename E>
    EntityID add(E entity);

    void remove(EntityID entity);

    void draw();
    void update();

    template <typename E>
    E *fetch(EntityID id);
};

///*
// Fetches an entity of the given type.
template <typename E>
E *EntitySystem::fetch(EntityID id) {
    ASSERT(is_valid(id), "Cannot fetch entity that doesn't exist");
    return dynamic_cast<E *>(entities[id]);
}

///*
// Adds a new entity to the entity system.
template <typename E>
EntityID EntitySystem::add(E entity) {
    EntityID id = next_id++;
    ASSERT(!is_valid(id), "Adding multiple entity ids for one id");
    E *e = new E();
    *e = entity;
    e->type = type_of(e);
    entities[id] = (BaseEntity *)e;
    e->on_create();
    return id;
}

///*
// Returns true if the given entity type has a field with the
// given name, note that this name has to be a FieldName::.
// TODO(ed): FieldName:: should be enforced via a typecheck.
bool has_field_by_name(BaseEntity *e, const char *);
///*
// A global getter for the entity_system.
EntitySystem *entity_system();
