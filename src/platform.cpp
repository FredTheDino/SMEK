#include <stdio.h>
#include "game.h"
#include "util/log.h"
#include "input.h"
#include <unordered_map>

// Returns the length of a statically allocated list.
#define LEN(a) (sizeof(a) / sizeof(a[0]))

// This is very UNIX-y
#include <dlfcn.h>
#include <sys/stat.h>

struct GameLibrary {
    GameInitFunc init;
    GameReloadFunc reload;
    GameUpdateFunc update;

    void *handle;

    int last_time;
    int reload_frame_delay;
} game_lib = {};

int get_file_edit_time(const char *path) {
    struct stat attr;
    // Check for success
    if (stat(path, &attr)) {
        return -1;
    }
    return attr.st_ctime;
}

bool load_gamelib() {
    GameLibrary next_library = {};
    //
    // TODO(ed): Check if RTLD_NODELETE lets you reference memory from old loaded DLLs. That would be
    // cool and potentially costly...
    const char *path = "./libSMEK.so";

    int edit_time = get_file_edit_time(path);
    if (edit_time == game_lib.last_time)
        return false;

    if (game_lib.reload_frame_delay) {
        game_lib.reload_frame_delay--;
        return false;
    }

    void *tmp = dlopen(path, RTLD_NOW);
    if (!tmp) {
        return false;
    }

    dlerror(); // Clear all errors.
    dlsym(tmp, "init_game");
    if (const char *error = dlerror()) {
        dlclose(tmp);
        WARN("Failed to load symbol. (%s)", error);
        return false;
    }
    dlclose(tmp); // If it isn't unloaded here, the same library is loaded.

    // TODO(ed): Add locks in when they are needed
    if (game_lib.handle) { dlclose(game_lib.handle); }

    void *lib = dlopen(path, RTLD_NOW);
    ASSERT(lib, "Failed to open library safely");

    game_lib.handle = lib;
    game_lib.init = (GameInitFunc) dlsym(lib, "init_game");
    if (!game_lib.init) {
        UNREACHABLE("Failed to load \"init_game\" (%s)", dlerror());
    }
    game_lib.update = (GameUpdateFunc) dlsym(lib, "update_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"init_update\" (%s)", dlerror());
    }
    game_lib.reload = (GameReloadFunc) dlsym(lib, "reload_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"init_update\" (%s)", dlerror());
    }

    game_lib.last_time = edit_time;
    game_lib.reload_frame_delay = 10;
    return true;
}

struct GameInput {
    using Button = i32;

    struct ButtonPress {
        Ac action;
        f32 value;
    };

    Button action_to_input[(u32) Ac::NUM_ACTIONS][2] = {};
    std::unordered_map<Button, ButtonPress> input_to_action;
    f32 state[(u32) Ac::NUM_ACTIONS] = {};

    bool rebinding;
    Ac rebind_action;
    u32 rebind_slot;
    f32 rebind_value;

    void bind(Ac action, u32 slot, Button button, f32 value=1.0) {
        ASSERT(slot < LEN(action_to_input[0]), "Invalid binding slot, max %d. (%d)",
               LEN(action_to_input[0]), slot);
        if (input_to_action.count(button))
            WARN("Button cannot be bound to multiple actions (%d)", button);
        action_to_input[(u32) action][slot] = button;
        input_to_action[button] = { action, value };
    }

    bool unbind(Ac action, u32 slot) {
        ASSERT(slot < LEN(action_to_input[0]), "Invalid binding slot, max %d. (%d)",
               LEN(action_to_input[0]), slot);

        Button button = action_to_input[(u32) action][slot];
        action_to_input[(u32) action][slot] = 0;

        if (button == 0) { return false; }
        input_to_action.erase(button);
        return true;
    }

    bool eaten_by_rebind(Button button) {
        if (!rebinding) return false;
        rebinding = false;
        unbind(rebind_action, rebind_slot);
        bind(rebind_action, rebind_slot, button, rebind_value);
        return true;
    }

    void update_press(Button button, bool down) {
        if (input_to_action.count(button)) {
            ButtonPress press = input_to_action[button];
            state[(u32) press.action] = down * press.value;
        }
    }
} global_input = {};

// See documentation in input.h
void platform_rebind(Ac action, u32 slot, f32 value) {
    ASSERT(slot < LEN(global_input.action_to_input[0]), "Invalid binding slot, max %d. (%d)",
           LEN(global_input.action_to_input[0]), slot);
    global_input.rebinding = true;
    global_input.rebind_action = action;
    global_input.rebind_slot = slot;
    global_input.rebind_value = value;
}

void platform_bind(Ac action, u32 slot, u32 button, f32 value) {
    global_input.unbind(action, slot);
    global_input.bind(action, slot, button, value);
}

#ifndef TESTS
#include "util/log.cpp" // I know, just meh.
int main() { // Game entrypoint
    if (!load_gamelib()) {
        UNREACHABLE("Failed to load the linked library the first time!");
    }
    GameState gs;
    gs.input.rebind_func = platform_rebind;
    gs.input.bind_func = platform_bind;

    game_lib.init(&gs);
    game_lib.reload(&gs);

    int frame = 0;
    const int RELOAD_TIME = 1; // Set this to a higher number to prevent constant disk-checks.

    while (gs.running) {
        // Check for reloading of library
        if (++frame == RELOAD_TIME) {
            frame = 0;
            if (load_gamelib()) {
                LOG("PLATFORM LAYER RELOAD!");
                game_lib.reload(&gs);
            }
        }

        // Read in input
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    gs.running = false;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                SDL_KeyboardEvent key = event.key;
                if (key.repeat) continue;
                GameInput::Button button = key.keysym.sym;
                bool down = key.state == SDL_PRESSED;
                if (down && global_input.eaten_by_rebind(button)) continue;
                global_input.update_press(button, down);
            }
        }

        for (u32 i = 0; i < (u32) Ac::NUM_ACTIONS; i++) {
            gs.input.last_frame[i] = gs.input.current_frame[i];
            gs.input.current_frame[i] = global_input.state[i];
        }

        gs = game_lib.update(&gs, GSUM::UPDATE_AND_RENDER);
    }
    return 0;
}
#endif

#define SOME_ACTION ((Ac) 0)

#define DEFAULT_INPUT_TEST \
    GameInput input;\
    const u32 button_1 = 2;\
    const u32 button_2 = 5;\
    const u32 button_3 = 232;\
    input.bind(SOME_ACTION, 0, button_1,  1.0);\
    input.bind(SOME_ACTION, 1, button_2, -1.0)

#include "test.h"
TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    return input.state[0] == 0.0;
});

TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    return input.state[0] == 1.0;
});

TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    input.update_press(button_1, false);
    input.update_press(button_3, true);
    input.update_press(button_1, true);
    return input.state[0] == 1.0;
});

TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    input.update_press(button_2, true);
    input.update_press(button_3, false);
    return input.state[0] == -1.0;
});

TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, false);
    input.update_press(button_1, false);
    input.update_press(button_1, false);
    return input.state[0] == 0.0;
});

TEST_CASE("platform_input_rebind", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    input.update_press(button_1, false);
    input.unbind(SOME_ACTION, 0);
    input.update_press(button_1, true);
    input.update_press(button_1, false);
    return input.state[0] == 0.0;
});

TEST_CASE("platform_input_rebind", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    input.update_press(button_3, false);
    input.unbind(SOME_ACTION, 0);
    input.bind(SOME_ACTION, 0, button_3);
    input.update_press(button_1, true);
    input.update_press(button_1, false);
    return input.state[0] == 0.0;
});

TEST_CASE("platform_input_rebind", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    input.update_press(button_3, false);
    input.unbind(SOME_ACTION, 0);
    input.bind(SOME_ACTION, 0, button_3);
    input.update_press(button_3, true);
    return input.state[0] == 1.0;
});

TEST_CASE("platform_input_rebind", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    input.unbind(SOME_ACTION, 0);
    input.update_press(button_1, false);
    input.bind(SOME_ACTION, 0, button_1);
    input.update_press(button_1, false);
    return input.state[0] == 0.0;
});
