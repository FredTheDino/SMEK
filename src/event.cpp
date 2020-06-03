#include "event.h"

#include "game.h"

namespace EventSystem {

void EventA::callback() {
    LOG("A: {}", a);
}

void handle_events() {
#define CASE(NAME) case NAME: e.NAME.callback(); break
    GameState *state = GAMESTATE();
    Event e;
    while (!state->event_queue.empty()) {
        e = state->event_queue.front();
        state->event_queue.pop();
        switch (e.type) {
        CASE(A);
        default:
            ERROR("Unkown event type {}", e.type);
        }
    }
}

}  // namespace EventSystem
