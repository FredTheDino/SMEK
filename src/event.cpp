#include "event.h"

#include "entity/entity.h"
#include "game.h"

namespace EventSystem {

void handle_events() {
#define HANDLE(NAME) case NAME: e.NAME.callback(); break
    GameState *state = GAMESTATE();
    Event e;
    while (!state->event_queue.empty()) {
        e = state->event_queue.front();
        state->event_queue.pop();
        switch (e.type) {
            HANDLE(CREATE_SOUND_ENTITY);
            HANDLE(CREATE_PLAYER);
            default:
                ERROR("Unkown event type {}", e.type);
        }
    }
#undef HANDLE
}

void EventCreatePlayer::callback() {
    Player player;
    GAMESTATE()->entity_system.add(player);
}

}  // namespace EventSystem
