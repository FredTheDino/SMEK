#include "event.h"

#include "entity/entity.h"
#include "game.h"

namespace EventSystem {

void handle_events() {
#define HANDLE(NAME) \
    case NAME: e.NAME.callback(); break
    GameState *state = GAMESTATE();
    Event e = {};
    while (!state->event_queue.empty()) {
        e = state->event_queue.front();
        state->event_queue.pop();
        switch (e.type) {
            HANDLE(CREATE_ENTITY);
        default:
            ERR("Unkown event type {}", e.type);
        }
    }
#undef HANDLE
}

} // namespace EventSystem
