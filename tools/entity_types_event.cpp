EventSystem::Event entity_event($entity_type entity) {
    EventSystem::Event event = {
        .type = EventSystem::EventType::CREATE_ENTITY,
        .CREATE_ENTITY = { .type = EntityType::$entity_type_enum }
    };
    std::memcpy(event.CREATE_ENTITY.${entity_type_enum}, ((void *)&entity) + sizeof(void *), sizeof($entity_type) - sizeof(void *));
    return event;
}
