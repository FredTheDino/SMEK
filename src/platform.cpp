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

    void bind(Ac action, u32 id, Button button, f32 value=1.0) {
        ASSERT(id < LEN(action_to_input[0]), "Invalid binding id, max %d. (%d)", LEN(action_to_input[0]), id);
        if (input_to_action.count(button))
            WARN("Button cannot be bound to multiple actions (%d)", button);
        action_to_input[(u32) action][id] = button;
        input_to_action[button] = { action, value };
    }

    void unbind(Ac action, u32 id) {
        ASSERT(id < LEN(action_to_input[0]), "Invalid binding id, max %d. (%d)", LEN(action_to_input[0]), id);

        Button button = action_to_input[(u32) action][id];
        action_to_input[(u32) action][id] = -1;

        if (button == -1) { WARN("Trying to unbind unbound button. (%d)", action); return; }
        input_to_action.erase(button);
    }

    void update_press(Button button, bool down) {
        if (input_to_action.count(button)) {
            ButtonPress press = input_to_action[button];
            state[(u32) press.action] = down * press.value;
        }
    }
};

#ifndef TESTS
#include "util/log.cpp" // I know, just meh.
int main() { // Game entrypoint
    if (!load_gamelib()) {
        UNREACHABLE("Failed to load the linked library the first time!");
    }
    GameState gs;
    GameInput input;

    game_lib.init(&gs);
    game_lib.reload(&gs);

    int frame = 0;
    const int RELOAD_TIME = 1; // Set this to a higher number to prevent constant disk-checks.

    input.bind(Ac::AButton, 0, SDLK_a, 1.0);
    input.bind(Ac::AButton, 1, SDLK_s, 0.1);
    input.bind(Ac::BButton, 0, SDLK_b, 1.0);
    input.bind(Ac::BButton, 1, SDLK_n, 0.1);

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
                input.update_press(button, key.state == SDL_PRESSED);
            }
        }

        for (u32 i = 0; i < (u32) Ac::NUM_ACTIONS; i++) {
            gs.input.last_frame[i] = gs.input.current_frame[i];
            gs.input.current_frame[i] = input.state[i];
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
TEST_CASE("platform_input", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    return input.state[0] == 0.0;
});

TEST_CASE("platform_input", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    return input.state[0] == 1.0;
});

TEST_CASE("platform_input", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    input.update_press(button_1, false);
    input.update_press(button_3, true);
    input.update_press(button_1, true);
    return input.state[0] == 1.0;
});

TEST_CASE("platform_input", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    input.update_press(button_2, true);
    input.update_press(button_3, false);
    return input.state[0] == -1.0;
});

TEST_CASE("platform_input", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, false);
    input.update_press(button_1, false);
    input.update_press(button_1, false);
    return input.state[0] == 0.0;
});
