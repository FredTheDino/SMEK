#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "glad/glad.h"

#include "game.h"
#include "util/log.h"
#include "input.h"
#include <unordered_map>
#include <mutex>
#include <csignal>

// Returns the length of a statically allocated list.
#define LEN(a) (sizeof(a) / sizeof(a[0]))
#include "math/smek_math.h"
#include "audio.h"

// This is very UNIX-y
#include <dlfcn.h>

struct GameLibrary {
    GameInitFunc init;
    GameReloadFunc reload;
    GameUpdateFunc update;
    AudioCallbackFunc audio_callback;

    void *handle;

    int last_time;
    int reload_frame_delay;
} game_lib = {};

GameState game_state;
Audio::AudioStruct platform_audio_struct = {};

std::mutex m_reload_lib;
bool reload_lib = false;

void signal_handler (int signal) {
    m_reload_lib.lock();
    reload_lib = true;
    m_reload_lib.unlock();
}

bool load_gamelib() {
    m_reload_lib.lock();
    if (!reload_lib) {
        m_reload_lib.unlock();
        return false;
    }
    reload_lib = false;
    m_reload_lib.unlock();

    GameLibrary next_library = {};
    //
    // TODO(ed): Check if RTLD_NODELETE lets you reference memory from old loaded DLLs. That would be
    // cool and potentially costly...
    const char *path = "./libSMEK.so";

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

    platform_audio_struct.lock();
    if (game_lib.handle) { dlclose(game_lib.handle); }

    void *lib = dlopen(path, RTLD_NOW);
    if (!lib) {
        UNREACHABLE("Failed to open library safely (%s)", dlerror());
    }

    game_lib.handle = lib;
    game_lib.init = (GameInitFunc) dlsym(lib, "init_game");
    if (!game_lib.init) {
        UNREACHABLE("Failed to load \"init_game\" (%s)", dlerror());
    }
    game_lib.update = (GameUpdateFunc) dlsym(lib, "update_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"update_game\" (%s)", dlerror());
    }
    game_lib.reload = (GameReloadFunc) dlsym(lib, "reload_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"reload_game\" (%s)", dlerror());
    }
    game_lib.audio_callback = (AudioCallbackFunc) dlsym(lib, "audio_callback");
    if (!game_lib.audio_callback) {
        UNREACHABLE("Failed to load \"audio_callback\" (%s)", dlerror());
    }
    platform_audio_struct.unlock();

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
    Vec2 mouse_pos;
    Vec2 mouse_move;

    bool capture_mouse;

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

void platform_audio_callback(void *userdata, u8 *stream, int len) {
    f32 *f_stream = (f32 *) stream;
    Audio::AudioStruct *audio_struct_ptr = (Audio::AudioStruct *) userdata;
    game_lib.audio_callback(audio_struct_ptr, f_stream, len);
}

void platform_audio_init() {
    platform_audio_struct = {};

    SDL_AudioSpec want = {};
    want.freq = 48000;
    want.format = AUDIO_F32;
    want.samples = 2048;
    want.channels = 2;
    want.callback = platform_audio_callback;
    want.userdata = (void *) &platform_audio_struct;

    SDL_AudioSpec have;
    platform_audio_struct.dev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (platform_audio_struct.dev <= 0) {
        UNREACHABLE("Unable to initialize audio (%s)", SDL_GetError());
    }
    CHECK(have.freq == want.freq, "Got different sample rate %d", have.freq);
    ASSERT(have.format == want.format, "Got wrong format %d", have.format);
    ASSERT(have.channels == want.channels, "Got wrong amount of channels %d", have.channels);

    platform_audio_struct.sample_rate = have.freq;
    platform_audio_struct.active = true;

    SDL_PauseAudioDevice(platform_audio_struct.dev, 0);
}

#ifndef TESTS
#include "util/log.cpp" // I know, just meh.
int main() { // Game entrypoint
    m_reload_lib.lock();
    reload_lib = true;
    m_reload_lib.unlock();

    if (!load_gamelib()) {
        UNREACHABLE("Failed to load the linked library the first time!");
    }
    game_state = {};
    game_state.input.mouse_capture = false;
    game_state.input.rebind_func = platform_rebind;
    game_state.input.bind_func = platform_bind;

    game_state.input.rebind_func = platform_rebind;
    game_state.input.bind_func = platform_bind;

    game_lib.init(&game_state);
    platform_audio_init();
    game_state.audio_struct = &platform_audio_struct;

    // IMGUI
    const char* glsl_version = "#version 330";
    if (gladLoadGL() == 0) {
        UNREACHABLE("Failed to load glad");
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    ImGui_ImplSDL2_InitForOpenGL(game_state.window, game_state.gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    game_lib.reload(&game_state);

    int frame = 0;
    const int RELOAD_TIME = 1; // Set this to a higher number to prevent constant disk-checks.

    std::signal(SIGUSR1, signal_handler);

    while (game_state.running) {
        // Check for reloading of library
        if (++frame == RELOAD_TIME) {
            frame = 0;
            if (load_gamelib()) {
                LOG("PLATFORM LAYER RELOAD!");
                game_lib.reload(&game_state);
            }
        }

        // Zero the movement, so we don't carry over frames.
        global_input.mouse_move = {};
        // Read in input
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                game_state.running = false;
            }
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    game_state.running = false;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                if (io.WantCaptureKeyboard) continue;
                SDL_KeyboardEvent key = event.key;
                if (key.repeat) continue;
                GameInput::Button button = key.keysym.sym;
                bool down = key.state == SDL_PRESSED;
                if (down && global_input.eaten_by_rebind(button)) continue;
                global_input.update_press(button, down);
            }
            if (io.WantCaptureMouse) continue;
            if (event.type == SDL_MOUSEMOTION) {
                SDL_MouseMotionEvent mouse = event.motion;
                global_input.mouse_move = Vec2(mouse.xrel, mouse.yrel);
                global_input.mouse_pos = Vec2(mouse.x, mouse.y);
            }
        }


        for (u32 i = 0; i < (u32) Ac::NUM_ACTIONS; i++) {
            game_state.input.last_frame[i] = game_state.input.current_frame[i];
            game_state.input.current_frame[i] = global_input.state[i];
        }
        game_state.input.mouse_move = global_input.mouse_move;
        game_state.input.mouse_pos = global_input.mouse_pos;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(game_state.window);
        ImGui::NewFrame();

        game_state = game_lib.update(&game_state, GSUM::UPDATE_AND_RENDER);

        SDL_SetRelativeMouseMode((SDL_bool) game_state.input.mouse_capture);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(game_state.window);
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
