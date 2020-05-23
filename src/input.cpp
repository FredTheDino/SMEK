#include "input.h"
#include "game.h"

namespace Input {

static f32 get_current(Action name) {
    return GAMESTATE()->input.current_frame[(u32) name];
}

static f32 get_last(Action name) {
    return GAMESTATE()->input.last_frame[(u32) name];
}

bool down(Action name) {
    return get_current(name) != 0.0;
}

bool up(Action name) {
    return get_current(name) == 0.0;
}

bool released(Action name) {
    return get_last(name) == 1.0 && up(name);
}

bool pressed(Action name) {
    return get_last(name) == 0.0 && down(name);
}

}
