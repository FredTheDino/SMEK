    case EntityType::$entity_type_enum: {
        LOG("unpacking $entity_type");
        $entity_type entity;
        std::memcpy(((u8 *)&entity) + sizeof(u8 *), $entity_type_enum, sizeof($entity_type) - sizeof(u8 *));
        GAMESTATE()->entity_system.add_with_id(entity, entity.entity_id);
        break;
    }
