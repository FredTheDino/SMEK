        case EntityType::$entity_type_enum: {
            $entity_type entity;
            std::memcpy(&entity, $entity_type_enum, sizeof($entity_type));
            GAMESTATE()->entity_system.add(entity);
            break;
        }
