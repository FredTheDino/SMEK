// clang-format off
/*
 * Do not edit this file directly! It is generated from
 * `tools/entity_types.cpp` by the build system at compile time.
 */
#include "entity_types.h"
#include "entity.h"

#include <cstring>
#include <stddef.h>
#include "../event.h"
#include "../game.h"

i32 format(char *buffer, u32 size, FormatHint args, EntityType type) {
    switch (type) {
    case EntityType::BASEENTITY: return snprintf(buffer, size, "BaseEntity");
    case EntityType::BLOCK: return snprintf(buffer, size, "Block");
    case EntityType::ENTITY: return snprintf(buffer, size, "Entity");
    case EntityType::LIGHT: return snprintf(buffer, size, "Light");
    case EntityType::PLAYER: return snprintf(buffer, size, "Player");
    case EntityType::SOUNDENTITY: return snprintf(buffer, size, "SoundEntity");
    default: return snprintf(buffer, size, "?ENTITY TYPE?");
    }
    return 0;
}

namespace FieldName {
FieldNameType asset_id = "asset_id";
FieldNameType audio_id = "audio_id";
FieldNameType color = "color";
FieldNameType draw_as_point = "draw_as_point";
FieldNameType entity_id = "entity_id";
FieldNameType hit = "hit";
FieldNameType hp = "hp";
FieldNameType last_input = "last_input";
FieldNameType light_id = "light_id";
FieldNameType position = "position";
FieldNameType remove = "remove";
FieldNameType rotation = "rotation";
FieldNameType scale = "scale";
FieldNameType sound_source_settings = "sound_source_settings";
FieldNameType type = "type";
FieldNameType velocity = "velocity";
};

Field gen_BaseEntity[] = {
    { typeid(bool), FieldName::remove, sizeof(bool), (int)offsetof(BaseEntity, remove), 0, },
    { typeid(EntityID), FieldName::entity_id, sizeof(EntityID), (int)offsetof(BaseEntity, entity_id), 1, },
    { typeid(EntityType), FieldName::type, sizeof(EntityType), (int)offsetof(BaseEntity, type), 1, }
};
Field gen_Block[] = {
    { typeid(bool), FieldName::remove, sizeof(bool), (int)offsetof(Block, remove), 0, },
    { typeid(EntityID), FieldName::entity_id, sizeof(EntityID), (int)offsetof(Block, entity_id), 1, },
    { typeid(EntityType), FieldName::type, sizeof(EntityType), (int)offsetof(Block, type), 1, },
    { typeid(Vec3), FieldName::position, sizeof(Vec3), (int)offsetof(Block, position), 0, },
    { typeid(Vec3), FieldName::scale, sizeof(Vec3), (int)offsetof(Block, scale), 0, },
    { typeid(Quat), FieldName::rotation, sizeof(Quat), (int)offsetof(Block, rotation), 0, }
};
Field gen_Entity[] = {
    { typeid(bool), FieldName::remove, sizeof(bool), (int)offsetof(Entity, remove), 0, },
    { typeid(EntityID), FieldName::entity_id, sizeof(EntityID), (int)offsetof(Entity, entity_id), 1, },
    { typeid(EntityType), FieldName::type, sizeof(EntityType), (int)offsetof(Entity, type), 1, },
    { typeid(Vec3), FieldName::position, sizeof(Vec3), (int)offsetof(Entity, position), 0, },
    { typeid(Vec3), FieldName::scale, sizeof(Vec3), (int)offsetof(Entity, scale), 0, },
    { typeid(Quat), FieldName::rotation, sizeof(Quat), (int)offsetof(Entity, rotation), 0, }
};
Field gen_Light[] = {
    { typeid(bool), FieldName::remove, sizeof(bool), (int)offsetof(Light, remove), 0, },
    { typeid(EntityID), FieldName::entity_id, sizeof(EntityID), (int)offsetof(Light, entity_id), 1, },
    { typeid(EntityType), FieldName::type, sizeof(EntityType), (int)offsetof(Light, type), 1, },
    { typeid(Vec3), FieldName::position, sizeof(Vec3), (int)offsetof(Light, position), 0, },
    { typeid(Vec3), FieldName::scale, sizeof(Vec3), (int)offsetof(Light, scale), 0, },
    { typeid(Quat), FieldName::rotation, sizeof(Quat), (int)offsetof(Light, rotation), 0, },
    { typeid(i32), FieldName::light_id, sizeof(i32), (int)offsetof(Light, light_id), 1, },
    { typeid(Color3), FieldName::color, sizeof(Color3), (int)offsetof(Light, color), 0, },
    { typeid(bool), FieldName::draw_as_point, sizeof(bool), (int)offsetof(Light, draw_as_point), 0, }
};
Field gen_Player[] = {
    { typeid(bool), FieldName::remove, sizeof(bool), (int)offsetof(Player, remove), 0, },
    { typeid(EntityID), FieldName::entity_id, sizeof(EntityID), (int)offsetof(Player, entity_id), 1, },
    { typeid(EntityType), FieldName::type, sizeof(EntityType), (int)offsetof(Player, type), 1, },
    { typeid(Vec3), FieldName::position, sizeof(Vec3), (int)offsetof(Player, position), 0, },
    { typeid(Vec3), FieldName::scale, sizeof(Vec3), (int)offsetof(Player, scale), 0, },
    { typeid(Quat), FieldName::rotation, sizeof(Quat), (int)offsetof(Player, rotation), 0, },
    { typeid(PlayerInput), FieldName::last_input, sizeof(PlayerInput), (int)offsetof(Player, last_input), 1, },
    { typeid(Vec3), FieldName::velocity, sizeof(Vec3), (int)offsetof(Player, velocity), 0, },
    { typeid(Physics::Manifold), FieldName::hit, sizeof(Physics::Manifold), (int)offsetof(Player, hit), 0, },
    { typeid(real), FieldName::hp, sizeof(real), (int)offsetof(Player, hp), 0, }
};
Field gen_SoundEntity[] = {
    { typeid(bool), FieldName::remove, sizeof(bool), (int)offsetof(SoundEntity, remove), 0, },
    { typeid(EntityID), FieldName::entity_id, sizeof(EntityID), (int)offsetof(SoundEntity, entity_id), 1, },
    { typeid(EntityType), FieldName::type, sizeof(EntityType), (int)offsetof(SoundEntity, type), 1, },
    { typeid(AssetID), FieldName::asset_id, sizeof(AssetID), (int)offsetof(SoundEntity, asset_id), 0, },
    { typeid(Audio::SoundSourceSettings), FieldName::sound_source_settings, sizeof(Audio::SoundSourceSettings), (int)offsetof(SoundEntity, sound_source_settings), 0, },
    { typeid(AudioID), FieldName::audio_id, sizeof(AudioID), (int)offsetof(SoundEntity, audio_id), 0, }
};

FieldList get_fields_for(EntityType type) {
    switch (type) {
    case EntityType::BASEENTITY:return { LEN(gen_BaseEntity), gen_BaseEntity };
    case EntityType::BLOCK:return { LEN(gen_Block), gen_Block };
    case EntityType::ENTITY:return { LEN(gen_Entity), gen_Entity };
    case EntityType::LIGHT:return { LEN(gen_Light), gen_Light };
    case EntityType::PLAYER:return { LEN(gen_Player), gen_Player };
    case EntityType::SOUNDENTITY:return { LEN(gen_SoundEntity), gen_SoundEntity };
    default:
        UNREACHABLE("Unknown entity type");
        return {};
    }
}

void emplace_entity(void *buffer, EntityType type) {
    switch (type) {
    case EntityType::BASEENTITY:
        *((BaseEntity *) buffer) = BaseEntity();
        ((BaseEntity *) buffer)->type = EntityType::BASEENTITY;
        return;
    case EntityType::BLOCK:
        *((Block *) buffer) = Block();
        ((Block *) buffer)->type = EntityType::BLOCK;
        return;
    case EntityType::ENTITY:
        *((Entity *) buffer) = Entity();
        ((Entity *) buffer)->type = EntityType::ENTITY;
        return;
    case EntityType::LIGHT:
        *((Light *) buffer) = Light();
        ((Light *) buffer)->type = EntityType::LIGHT;
        return;
    case EntityType::PLAYER:
        *((Player *) buffer) = Player();
        ((Player *) buffer)->type = EntityType::PLAYER;
        return;
    case EntityType::SOUNDENTITY:
        *((SoundEntity *) buffer) = SoundEntity();
        ((SoundEntity *) buffer)->type = EntityType::SOUNDENTITY;
        return;
    default:
        UNREACHABLE("Unknown entity type");
    }
}

EntityID EntitySystem::add_unknown_type(BaseEntity *e) {
    switch (e->type) {
    case EntityType::BASEENTITY:
        return add<BaseEntity>(*(BaseEntity *) e);
    case EntityType::BLOCK:
        return add<Block>(*(Block *) e);
    case EntityType::ENTITY:
        return add<Entity>(*(Entity *) e);
    case EntityType::LIGHT:
        return add<Light>(*(Light *) e);
    case EntityType::PLAYER:
        return add<Player>(*(Player *) e);
    case EntityType::SOUNDENTITY:
        return add<SoundEntity>(*(SoundEntity *) e);
    default:
        UNREACHABLE("Unknown entity type");
        return {};
    }
}

/*
 * Included from `tools/entity_types_type_of.cpp`
 */

EntityType type_of(BaseEntity *e) {
    return EntityType::BASEENTITY;
}

EntityType type_of(Block *e) {
    return EntityType::BLOCK;
}

EntityType type_of(Entity *e) {
    return EntityType::ENTITY;
}

EntityType type_of(Light *e) {
    return EntityType::LIGHT;
}

EntityType type_of(Player *e) {
    return EntityType::PLAYER;
}

EntityType type_of(SoundEntity *e) {
    return EntityType::SOUNDENTITY;
}

/*
 * End of `tools/entity_types_type_of.cpp`
 */

/*
 * Included from `tools/entity_types_event_callback.cpp`
 */

void EventCreateEntity::callback() {
    switch (type) {
    case EntityType::BASEENTITY: {
        BaseEntity entity;
        std::memcpy(((u8 *)&entity) + sizeof(u8 *), BASEENTITY, sizeof(BaseEntity) - sizeof(u8 *));
        if (generate_id) {
            GAMESTATE()->entity_system.add(entity);
        } else {
            GAMESTATE()->entity_system.add_with_id(entity, entity.entity_id);
        }
        break;
    }
    case EntityType::BLOCK: {
        Block entity;
        std::memcpy(((u8 *)&entity) + sizeof(u8 *), BLOCK, sizeof(Block) - sizeof(u8 *));
        if (generate_id) {
            GAMESTATE()->entity_system.add(entity);
        } else {
            GAMESTATE()->entity_system.add_with_id(entity, entity.entity_id);
        }
        break;
    }
    case EntityType::ENTITY: {
        Entity entity;
        std::memcpy(((u8 *)&entity) + sizeof(u8 *), ENTITY, sizeof(Entity) - sizeof(u8 *));
        if (generate_id) {
            GAMESTATE()->entity_system.add(entity);
        } else {
            GAMESTATE()->entity_system.add_with_id(entity, entity.entity_id);
        }
        break;
    }
    case EntityType::LIGHT: {
        Light entity;
        std::memcpy(((u8 *)&entity) + sizeof(u8 *), LIGHT, sizeof(Light) - sizeof(u8 *));
        if (generate_id) {
            GAMESTATE()->entity_system.add(entity);
        } else {
            GAMESTATE()->entity_system.add_with_id(entity, entity.entity_id);
        }
        break;
    }
    case EntityType::PLAYER: {
        Player entity;
        std::memcpy(((u8 *)&entity) + sizeof(u8 *), PLAYER, sizeof(Player) - sizeof(u8 *));
        if (generate_id) {
            GAMESTATE()->entity_system.add(entity);
        } else {
            GAMESTATE()->entity_system.add_with_id(entity, entity.entity_id);
        }
        break;
    }
    case EntityType::SOUNDENTITY: {
        SoundEntity entity;
        std::memcpy(((u8 *)&entity) + sizeof(u8 *), SOUNDENTITY, sizeof(SoundEntity) - sizeof(u8 *));
        if (generate_id) {
            GAMESTATE()->entity_system.add(entity);
        } else {
            GAMESTATE()->entity_system.add_with_id(entity, entity.entity_id);
        }
        break;
    }

    default:
        UNREACHABLE("Unknown entity type");
        break;
    }
}

/*
 * End of `tools/entity_types_event_callback.cpp`
 */

/*
 * Included from `tools/entity_types_event.cpp`
 */

Event entity_event(BaseEntity entity, bool generate_id) {
    return entity_event(&entity, generate_id);
}

Event entity_event(BaseEntity *entity, bool generate_id) {
    Event event = {
        .type = EventType::CREATE_ENTITY,
        .CREATE_ENTITY = {
            .generate_id = generate_id,
            .type = EntityType::BASEENTITY,
        }
    };
    std::memcpy(event.CREATE_ENTITY.BASEENTITY, ((u8 *)entity) + sizeof(u8 *), sizeof(BaseEntity) - sizeof(u8 *));
    return event;
}

Event entity_event(Block entity, bool generate_id) {
    return entity_event(&entity, generate_id);
}

Event entity_event(Block *entity, bool generate_id) {
    Event event = {
        .type = EventType::CREATE_ENTITY,
        .CREATE_ENTITY = {
            .generate_id = generate_id,
            .type = EntityType::BLOCK,
        }
    };
    std::memcpy(event.CREATE_ENTITY.BLOCK, ((u8 *)entity) + sizeof(u8 *), sizeof(Block) - sizeof(u8 *));
    return event;
}

Event entity_event(Entity entity, bool generate_id) {
    return entity_event(&entity, generate_id);
}

Event entity_event(Entity *entity, bool generate_id) {
    Event event = {
        .type = EventType::CREATE_ENTITY,
        .CREATE_ENTITY = {
            .generate_id = generate_id,
            .type = EntityType::ENTITY,
        }
    };
    std::memcpy(event.CREATE_ENTITY.ENTITY, ((u8 *)entity) + sizeof(u8 *), sizeof(Entity) - sizeof(u8 *));
    return event;
}

Event entity_event(Light entity, bool generate_id) {
    return entity_event(&entity, generate_id);
}

Event entity_event(Light *entity, bool generate_id) {
    Event event = {
        .type = EventType::CREATE_ENTITY,
        .CREATE_ENTITY = {
            .generate_id = generate_id,
            .type = EntityType::LIGHT,
        }
    };
    std::memcpy(event.CREATE_ENTITY.LIGHT, ((u8 *)entity) + sizeof(u8 *), sizeof(Light) - sizeof(u8 *));
    return event;
}

Event entity_event(Player entity, bool generate_id) {
    return entity_event(&entity, generate_id);
}

Event entity_event(Player *entity, bool generate_id) {
    Event event = {
        .type = EventType::CREATE_ENTITY,
        .CREATE_ENTITY = {
            .generate_id = generate_id,
            .type = EntityType::PLAYER,
        }
    };
    std::memcpy(event.CREATE_ENTITY.PLAYER, ((u8 *)entity) + sizeof(u8 *), sizeof(Player) - sizeof(u8 *));
    return event;
}

Event entity_event(SoundEntity entity, bool generate_id) {
    return entity_event(&entity, generate_id);
}

Event entity_event(SoundEntity *entity, bool generate_id) {
    Event event = {
        .type = EventType::CREATE_ENTITY,
        .CREATE_ENTITY = {
            .generate_id = generate_id,
            .type = EntityType::SOUNDENTITY,
        }
    };
    std::memcpy(event.CREATE_ENTITY.SOUNDENTITY, ((u8 *)entity) + sizeof(u8 *), sizeof(SoundEntity) - sizeof(u8 *));
    return event;
}

/*
 * End of `tools/entity_types_event.cpp`
 */
