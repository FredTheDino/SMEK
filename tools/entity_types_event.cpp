Event entity_event($entity_type entity) {
    Event event = {
        .type = EventType::CREATE_ENTITY,
        .CREATE_ENTITY = { .type = EntityType::$entity_type_enum }
    };
    std::memcpy(event.CREATE_ENTITY.${entity_type_enum}, ((void *)&entity) + sizeof(void *), sizeof($entity_type) - sizeof(void *));
    return event;
}
