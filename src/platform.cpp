#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "glad/glad.h"

#include "game.h"
#include "util/log.h"
#include "input.h"
#include <unordered_map>
#include <csignal>
#include <cstring>
#include <cstdlib>

#include "math/smek_math.h"
#include "audio.h"

#include "platform_input.h"

// This is very UNIX-y
#include <dlfcn.h>

#define MS_PER_FRAME 100

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

SDL_mutex *m_reload_lib;
bool hot_reload_active = true;
bool reload_lib = false;

void signal_handler (int signal) {
    if (!hot_reload_active) {
        WARN("Ignoring USR1");
    } else {
        if (SDL_LockMutex(m_reload_lib) == 0) {
            reload_lib = true;
            SDL_UnlockMutex(m_reload_lib);
        } else {
            ERROR("Unable to lock mutex: {}", SDL_GetError());
        }
    }
}

bool load_gamelib() {
    if (hot_reload_active) {
        if (SDL_LockMutex(m_reload_lib) != 0) {
            ERROR("Unable to lock mutex: {}", SDL_GetError());
            return false;
        } else {
            if (!reload_lib) {
                SDL_UnlockMutex(m_reload_lib);
                return false;
            } else {
                reload_lib = false;
                SDL_UnlockMutex(m_reload_lib);
            }
        }
    }

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
        WARN("Failed to load symbol: {}", error);
        return false;
    }
    dlclose(tmp); // If it isn't unloaded here, the same library is loaded.

    platform_audio_struct.lock();
    if (game_lib.handle) { dlclose(game_lib.handle); }

    void *lib = dlopen(path, RTLD_NOW);
    if (!lib) {
        UNREACHABLE("Failed to open library safely: {}", dlerror());
    }

    game_lib.handle = lib;
    game_lib.init = (GameInitFunc) dlsym(lib, "init_game");
    if (!game_lib.init) {
        UNREACHABLE("Failed to load \"init_game\": {}", dlerror());
    }
    game_lib.update = (GameUpdateFunc) dlsym(lib, "update_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"update_game\": {}", dlerror());
    }
    game_lib.reload = (GameReloadFunc) dlsym(lib, "reload_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"reload_game\": {}", dlerror());
    }
    game_lib.audio_callback = (AudioCallbackFunc) dlsym(lib, "audio_callback");
    if (!game_lib.audio_callback) {
        UNREACHABLE("Failed to load \"audio_callback\": {}", dlerror());
    }
    platform_audio_struct.unlock();

    return true;
}

///
// The function called by SDL, which calls our function.
void platform_audio_callback(void *userdata, u8 *stream, int len);

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
        UNREACHABLE("Unable to initialize audio: {}", SDL_GetError());
    }
    CHECK(have.freq == want.freq, "Got different sample rate ({})", have.freq);
    ASSERT(have.format == want.format, "Got wrong format ({})", have.format);
    ASSERT(have.channels == want.channels, "Got wrong amount of channels ({})", have.channels);

    platform_audio_struct.sample_rate = have.freq;
    platform_audio_struct.active = true;

    SDL_PauseAudioDevice(platform_audio_struct.dev, 0);
}

#ifndef TESTS
#include "util/log.cpp" // I know, just meh.
int main(int argc, char **argv) { // Game entrypoint
#define ARGUMENT(LONG, SHORT) (std::strcmp((LONG), argv[index]) == 0 || std::strcmp((SHORT), argv[index]) == 0)
    int width = 500;
    int height = 500;
    for (int index = 1; index < argc; index++) {
        if ARGUMENT("--help", "-h") {
            //TODO(gu)
            std::printf("Usage: SMEK [--help] [--resolution <width> <height>]\n"
                        "            [--no-reload]\n");
            return 0;
        } else if ARGUMENT("--resolution", "-r") {
            width = std::atoi(argv[++index]);
            height = std::atoi(argv[++index]);
        } else if ARGUMENT("--no-reload", "-R") {
            hot_reload_active = false;
        } else {
            ERROR("Unknown command line argument '{}'", argv[index]);
        }
    }
#undef ARGUMENT


    if (hot_reload_active) {
        m_reload_lib = SDL_CreateMutex();
        ASSERT(m_reload_lib, "Unable to create mutex");
        if (SDL_LockMutex(m_reload_lib) != 0) {
            ERROR("Unable to lock mutex: {}", SDL_GetError());
        } else {
            reload_lib = true;
            SDL_UnlockMutex(m_reload_lib);
        }
    }

    if (!load_gamelib()) {
        UNREACHABLE("Failed to load the linked library the first time!");
    }

    game_state = {};
    game_state.input.mouse_capture = false;
    game_state.input.rebind_func = platform_rebind;
    game_state.input.bind_func = platform_bind;
    game_state.input.rebind_func = platform_rebind;
    game_state.input.bind_func = platform_bind;

    game_lib.init(&game_state, width, height);
    platform_audio_init();
    game_state.audio_struct = &platform_audio_struct;

    // IMGUI
    if (gladLoadGL() == 0) {
        UNREACHABLE("Failed to load glad");
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Enable Keyboard and gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(game_state.window, game_state.gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    game_lib.reload(&game_state);

    std::signal(SIGUSR1, signal_handler);
    u32 next_update = SDL_GetTicks();
    while (game_state.running) {

        // Check for reloading of library
        if (load_gamelib()) {
            LOG("PLATFORM LAYER RELOAD!");
            game_lib.reload(&game_state);
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
                if (!game_state.input.mouse_capture) continue;
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

        while (next_update < SDL_GetTicks()) {
            game_state.time = next_update / 1000.0;
            game_state.delta = MS_PER_FRAME / 1000.0;
            game_state.frame++;
            next_update += MS_PER_FRAME;

            game_state = game_lib.update(&game_state, GSUM::UPDATE);
        }

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

//
// Tests are moved to a different file.
//
#include "platform_tests.h"
