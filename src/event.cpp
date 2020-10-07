#include "event.h"

#include "game.h"

void handle_events() {
#define HANDLE(NAME) \
    case NAME: e.NAME.callback(); break

    for (;;) {
        ASSERT(SDL_LockMutex(GAMESTATE()->m_event_queue) == 0, "Failed to aquire lock");
        if (GAMESTATE()->event_queue.empty()) {
            SDL_UnlockMutex(GAMESTATE()->m_event_queue);
            break;
        }
        Event e = GAMESTATE()->event_queue.front();
        GAMESTATE()->event_queue.pop();
        SDL_UnlockMutex(GAMESTATE()->m_event_queue);
        switch (e.type) {
            HANDLE(CREATE_ENTITY);
            HANDLE(LIGHT_UPDATE);
        default:
            ERR("Unkown event type {}", e.type);
        }
    }
#undef HANDLE
}
