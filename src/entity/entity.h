#pragma once
#include <unordered_map>
#include <set>
#include "../math/smek_math.h"
#include "../math/smek_vec.h"
#include "../math/smek_quat.h"
#include "../audio.h"

///# Entity system
//

using EntityID = u64;
#define INTERNAL

///* EntityType
// A magical enum that is automatically generated by the
// build script. Parses the source code to generate this.
enum class EntityType;

///* BaseEntity
// The base entity all other entities inherit from.
struct BaseEntity {
    bool remove = false;
    INTERNAL EntityID entity_id;

    virtual void imgui() {};

    virtual ~BaseEntity() {};
    virtual void update() {};
    virtual void draw() {};
    virtual void on_create() {};
    virtual void on_remove() {};

    INTERNAL EntityType type;
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

struct Block : public Entity {
    void on_create() override;
    void draw() override;
};

///* Light
// A light source in the world. Interfaces with the
// renderer under the hood and abstracts away the interface.
struct Light : public Entity {
    static const int NONE = -1;

    INTERNAL i32 light_id = NONE;
    Color3 color = Color3(0, 0, 0);

    bool draw_as_point;

    void imgui() override;

    void update() override;
    void draw() override;
    void on_remove() override;
};

struct LightUpdate {
    EntityID entity_id;
    real position[3];
    real color[3];
    bool draw_as_point;
    void callback();
};

struct PlayerInput {
    EntityID entity_id;
    f32 rotation[4];
    f32 move_axis[3];
    bool jump;
    bool shot;
    void callback();
};

///*
// A playable character
struct Player : public Entity {
    static constexpr f32 FLOOR = 0.2;
    INTERNAL PlayerInput last_input;
    Vec3 velocity;

    void imgui() override;

    void on_create() override;
    void update() override;
    void update_camera();
    void update_input();
    void update_position();
    void draw() override;
};

struct PlayerUpdate {
    EntityID entity_id;
    real position[3];
    real rotation[4];
    void callback();
};

struct ServerHandle;
struct ClientHandle;
///*
struct EntitySystem {
    static const u64 ID_MASK = 0x00FFFFFFFFFFFFFF;
    static const u64 CLIENT_MASK = 0xFF00000000000000;
    SDL_mutex *m_client_id;
    u64 client_id = 0;
    u64 id_counter = 0;
    u64 next_id();
    std::unordered_map<EntityID, BaseEntity *> entities;
    std::set<EntityID> selected;

    bool is_valid(EntityID id);
    bool have_ownership(EntityID id);

    template <typename E>
    EntityID add_with_id(E entity, EntityID id);

    template <typename E>
    EntityID add(E entity) { return add_with_id(entity, next_id()); }

    EntityID add_unknown_type(BaseEntity *e);

    void remove(EntityID entity);
    void remove_all();

    void draw_imgui();
    void draw();
    void send_state(ServerHandle *handle);
    void send_state(ClientHandle *handle);
    void send_initial_state(ClientHandle *handle);
    void update();

    // Remove all entities owned by client_id
    void drop_client(u64 client_id);

    template <typename E>
    E *fetch(EntityID id);
};

///*
// Fetches an entity of the given type.
template <typename E>
E *EntitySystem::fetch(EntityID id) {
    ASSERT(is_valid(id), "Fetching invalid entity id {}", id);
    return dynamic_cast<E *>(entities[id]);
}

///*
// Adds a new entity to the entity system.
template <typename E>
EntityID EntitySystem::add_with_id(E entity, EntityID id) {
    ASSERT(!is_valid(id), "Adding multiple entities for id {}", id);
    TRACE("Adding with id {}", id);
    E *e = new E();
    *e = entity;
    e->type = type_of(e);
    e->entity_id = id;
    entities[id] = (BaseEntity *)e;
    e->on_create();
    return id;
}

using FieldNameType = const char *;
///*
// Returns true if the given entity type has a field with the
// given name. Note that this name has to be a <code>FieldName::*</code>.
bool has_field_by_name(BaseEntity *e, FieldNameType name);

///*
// Helper function for fetching the field, it is returned without type,
// so this function can be used inside templates.
void *_fetch_field_by_name_helper(BaseEntity *e, FieldNameType name, const std::type_info &info);

///*
// Returns the name of the entity's type.
const char *type_name(BaseEntity *e);

template <typename F>
F *get_field_by_name_no_warn(BaseEntity *e, FieldNameType name) {
    return (F *)_fetch_field_by_name_helper(e, name, typeid(F));
}

///*
// Returns a field value if it has the named field and the
// named type.
template <typename F>
F *get_field_by_name(BaseEntity *e, FieldNameType name) {
    F *data = get_field_by_name_no_warn<F>(e, name);
    if (!data) {
        WARN("EntityType {} doesn't have '{}':'{}'.",
             type_name(e),
             name,
             typeid(F).name());
    }
    return data;
}

///*
// A global getter for the entity_system.
EntitySystem *entity_system();
