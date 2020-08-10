        case EntityType::$entity_type_enum: {
            $entity_type entity;
            std::memcpy(((void *) &entity) + sizeof(void *), $entity_type_enum, sizeof($entity_type) - sizeof(void *));
            GAMESTATE()->entity_system.add(entity);
            break;
        }
