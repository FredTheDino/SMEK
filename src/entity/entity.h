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
    EntityID entity_id;

    virtual ~BaseEntity() {};
    virtual void update() {};
    virtual void draw() {};
    virtual void on_create() {};
    virtual void on_remove() {};

#ifdef IMGUI_ENABLE
    virtual void imgui_create() {};
#endif

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
struct Light : public Entity {
    static const int NONE = -1;

    i32 light_id = NONE;
    Vec3 color;

    void update() override;
    void draw() override;
    void on_remove() override;

#ifdef IMGUI_ENABLE
    void imgui_create() override;
#endif
};

///*
// A playable character
struct Player : public Entity {
    Vec3 velocity;

    void update() override;
    void draw() override;
};

struct ClientHandle;
///*
struct EntitySystem {
    static const u64 ID_MASK = 0x00FFFFFFFFFFFFFF;
    SDL_mutex *m_client_id;
    u64 client_id = 0;
    u64 id_counter = 0;
    u64 next_id();
    std::unordered_map<EntityID, BaseEntity *> entities;

    bool is_valid(EntityID id);

    template <typename E>
    EntityID add(E entity) { return add_with_id(entity, next_id()); }

    template <typename E>
    EntityID add_with_id(E entity, EntityID id);

    void remove(EntityID entity);
    void remove_all();

    void draw();
    void send_state();
    void send_initial_state(ClientHandle *handle);
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
EntityID EntitySystem::add_with_id(E entity, EntityID id) {
    ASSERT(!is_valid(id), "Adding multiple entity ids for one id");
    LOG("Adding with id {}", id);
    E *e = new E();
    *e = entity;
    e->type = type_of(e);
    e->entity_id = id;
    entities[id] = (BaseEntity *)e;
    e->on_create();
    return id;
}

///*
// A global getter for the entity_system.
EntitySystem *entity_system();
