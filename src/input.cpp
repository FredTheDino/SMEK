#include "input.h"
#include "game.h"
#include "test.h"

namespace Input {

void rebind(Action name, u32 slot, f32 value) {
    GAMESTATE()->input.rebind_func(name, slot, value);
}

void bind(Action name, u32 slot, u32 button, f32 value) {
    GAMESTATE()->input.bind_func(name, slot, button, value);
}

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

f32 value(Action name) {
    return get_current(name);
}

#define SOME_ACTION ((Ac) 0)

TEST_CASE("input_down", {
    f32 *current = game->input.current_frame;
    f32 *last = game->input.last_frame;
    *current = 1.0;
    *last = 0.0;
    bool success = true;
    success &= down(SOME_ACTION);
    *current = 0.0;
    success &= !down(SOME_ACTION);
    return success;
});

TEST_CASE("input_up", {
    f32 *current = game->input.current_frame;
    f32 *last = game->input.last_frame;
    *current = 0.0;
    *last = 1.0;
    bool success = true;
    success &= up(SOME_ACTION);
    *current = 1.0;
    success &= !up(SOME_ACTION);
    return success;
});

TEST_CASE("input_released", {
    f32 *current = game->input.current_frame;
    f32 *last = game->input.last_frame;
    *current = 0.0;
    *last = 0.0;
    bool success = true;
    success &= !released(SOME_ACTION);
    *last = 1.0;
    success &= released(SOME_ACTION);
    return true;
});

TEST_CASE("input_pressed", {
    f32 *current = game->input.current_frame;
    f32 *last = game->input.last_frame;
    *current = 1.0;
    *last = 0.0;
    bool success = true;
    success &= pressed(SOME_ACTION);
    *last = 1.0;
    success &= !pressed(SOME_ACTION);
    return true;
});

}
