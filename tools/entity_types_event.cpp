Event entity_event($entity_type entity, bool generate_id) {
    return entity_event(&entity, generate_id);
}

Event entity_event($entity_type *entity, bool generate_id) {
    Event event = {
        .type = EventType::CREATE_ENTITY,
        .CREATE_ENTITY = {
            .generate_id = generate_id,
            .type = EntityType::$entity_type_enum
        }
    };
    std::memcpy(event.CREATE_ENTITY.${entity_type_enum}, ((u8 *)entity) + sizeof(u8 *), sizeof($entity_type) - sizeof(u8 *));
    return event;
}
