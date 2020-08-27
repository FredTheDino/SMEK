EventSystem::Event entity_event($entity_type entity) {
    EventSystem::Event event = {
        .type = EventSystem::EventType::CREATE_ENTITY,
        .CREATE_ENTITY = { .type = EntityType::$entity_type_enum }
    };
    std::memcpy(event.CREATE_ENTITY.${entity_type_enum}, ((u8 *)&entity) + sizeof(u8 *), sizeof($entity_type) - sizeof(u8 *));
    return event;
}
