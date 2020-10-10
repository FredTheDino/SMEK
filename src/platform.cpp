#ifdef IMGUI_ENABLE
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#endif

#include "glad/glad.h"
#include "game.h"
#include "math/smek_math.h"
#include "util/log.h"
#include "input.h"
#include "audio.h"

#include "platform_input.h"

#ifdef WINDOWS
#include "dlfcn-mingw.h"
#else
#include <dlfcn.h>
#endif

// This is very UNIX-y
#include <unordered_map>
#include <csignal>
#include <cstring>
#include <cstdlib>

#define MS_PER_FRAME 8

struct GameLibrary {
    GameInitFunc init;
    GameReloadFunc reload;
    GameUpdateFunc update;
    GameShutdownFunc shutdown;
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

#ifndef WINDOWS
void signal_handler(int signal) {
    if (!hot_reload_active) {
        WARN("Ignoring USR1");
    } else {
        if (SDL_LockMutex(m_reload_lib) == 0) {
            reload_lib = true;
            SDL_UnlockMutex(m_reload_lib);
        } else {
            ERR("Unable to lock mutex: {}", SDL_GetError());
        }
    }
}
#endif

// These macros are needed... Because...
#define HELPER(EXPR) "" STR(EXPR) ""
const char *game_lib_path = HELPER(SMEK_GAME_LIB);
#undef HELPER

bool load_gamelib() {
    if (hot_reload_active) {
        if (SDL_LockMutex(m_reload_lib) != 0) {
            ERR("Unable to lock mutex: {}", SDL_GetError());
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

    dlerror(); // Clear all errors.
    void *tmp = dlopen(game_lib_path, RTLD_NOW);
    if (!tmp) {
        WARN("Failed to open game lib: {}", dlerror());
        return false;
    }

    dlsym(tmp, "init_game");
    if (const char *error = dlerror()) {
        dlclose(tmp);
        WARN("Failed to load symbol: {}", error);
        return false;
    }
    dlclose(tmp); // If it isn't unloaded here, the same library is loaded.

    platform_audio_struct.lock();
    if (game_lib.handle) { dlclose(game_lib.handle); }

    void *lib = dlopen(game_lib_path, RTLD_NOW);
    if (!lib) {
        UNREACHABLE("Failed to open library safely: {}", dlerror());
    }

    game_lib.handle = lib;
    game_lib.init = (GameInitFunc)dlsym(lib, "init_game");
    if (!game_lib.init) {
        UNREACHABLE("Failed to load \"init_game\": {}", dlerror());
    }
    game_lib.update = (GameUpdateFunc)dlsym(lib, "update_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"update_game\": {}", dlerror());
    }
    game_lib.reload = (GameReloadFunc)dlsym(lib, "reload_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"reload_game\": {}", dlerror());
    }
    game_lib.shutdown = (GameShutdownFunc)dlsym(lib, "shutdown_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"shutdown_game\": {}", dlerror());
    }
    game_lib.audio_callback = (AudioCallbackFunc)dlsym(lib, "audio_callback");
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
    f32 *f_stream = (f32 *)stream;
    Audio::AudioStruct *audio_struct_ptr = (Audio::AudioStruct *)userdata;
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
    want.userdata = (void *)&platform_audio_struct;

    SDL_AudioSpec have;
    platform_audio_struct.dev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (platform_audio_struct.dev <= 0) {
        UNREACHABLE("Unable to initialize audio: {}", SDL_GetError());
    }
    CHECK(have.freq == want.freq, "Got different sample rate ({})", have.freq);
    ASSERT_EQ(have.format, want.format);
    ASSERT_EQ(have.channels, want.channels);

    platform_audio_struct.sample_rate = have.freq;
    platform_audio_struct.active = true;

    SDL_PauseAudioDevice(platform_audio_struct.dev, 0);
}

#ifdef IMGUI_ENABLE
static void imgui_platform_start() {
    IMGUI_CHECKVERSION();
    game_state.imgui.context = (void *)ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // Enable Keyboard and gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(game_state.window, game_state.gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");
}

static bool imgui_eats_input(SDL_Event *event) {
    ImGui_ImplSDL2_ProcessEvent(event);

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    if ((event->type == SDL_KEYDOWN
         || event->type == SDL_KEYUP)
        && io.WantCaptureKeyboard)
        return true;

    if ((event->type == SDL_MOUSEMOTION
         || event->type == SDL_MOUSEBUTTONDOWN
         || event->type == SDL_MOUSEBUTTONUP
         || event->type == SDL_MOUSEWHEEL)
        && io.WantCaptureMouse)
        return true;
    return false;
}

static void imgui_start_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(game_state.window);
    ImGui::NewFrame();
}

static void imgui_end_frame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    SDL_GL_MakeCurrent(game_state.window, game_state.gl_context);
}
#else
static void imgui_platform_start() {}
static constexpr bool imgui_eats_input(SDL_Event *event) { return false; }
static void imgui_start_frame() {}
static void imgui_end_frame() {}
#endif

#ifndef TESTS
bool update_to_default_window_size(int *width, int *height) {
    f32 screen_percent = 0.75;
    SDL_DisplayMode dm;
    // TODO(ed): First argument is display, maybe try multiple?
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        *width = dm.w * screen_percent;
        *height = dm.h * screen_percent;
        return true;
    }
    WARN("Failed to read Desktop Display Mode");
    if (SDL_GetCurrentDisplayMode(0, &dm) == 0) {
        *width = dm.w * screen_percent;
        *height = dm.h * screen_percent;
        return true;
    }
    WARN("Failed to read Current Display Mode");
    return false;
}

#include "util/log.cpp"           // I know, just meh.
int main(int argc, char **argv) { // Game entrypoint
    int width = 500;
    int height = 500;
    bool passed_resolution = false;
#define ARGUMENT(LONG, SHORT) (std::strcmp((LONG), argv[index]) == 0 || std::strcmp((SHORT), argv[index]) == 0)
    for (int index = 1; index < argc; index++) {
        if ARGUMENT ("--help", "-h") {
            std::printf("Usage: SMEK [--help] [--resolution <width> <height>]\n"
                        "            [--no-reload]\n");
            return 0;
        } else if ARGUMENT ("--resolution", "-r") {
            width = std::atoi(argv[++index]);
            height = std::atoi(argv[++index]);
            passed_resolution = true;
        } else if ARGUMENT ("--no-reload", "-R") {
            hot_reload_active = false;
        } else {
            ERR("Unknown command line argument '{}'", argv[index]);
        }
    }
#undef ARGUMENT

    if (hot_reload_active) {
        m_reload_lib = SDL_CreateMutex();
        ASSERT(m_reload_lib, "Unable to create mutex");
        if (SDL_LockMutex(m_reload_lib) != 0) {
            ERR("Unable to lock mutex: {}", SDL_GetError());
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
    game_state.m_reload_lib = m_reload_lib;
    game_state.reload_lib = &reload_lib;

    game_lib.init(&game_state, width, height);
    platform_audio_init();
    game_state.audio_struct = &platform_audio_struct;

    if (!passed_resolution && update_to_default_window_size(&width, &height)) {
        game_state.renderer.width = width;
        game_state.renderer.height = height;
        game_state.resized_window = true;
    }

    // IMGUI
    if (gladLoadGL() == 0) {
        UNREACHABLE("Failed to load glad");
    }

    imgui_platform_start();

    game_lib.reload(&game_state);

#ifndef WINDOWS
    std::signal(SIGUSR1, signal_handler);
#endif
    u32 next_update = SDL_GetTicks() - MS_PER_FRAME;
    while (game_state.running) {
        if (load_gamelib()) {
            LOG("PLATFORM LAYER RELOAD!");
            game_lib.reload(&game_state);
        }

        while (next_update < SDL_GetTicks() && game_state.running) {
            game_state.time = next_update / 1000.0;
            game_state.delta = MS_PER_FRAME / 1000.0;
            game_state.frame++;
            next_update += MS_PER_FRAME;

            // Zero the movement, so we don't carry over frames.
            global_input.mouse_move = {};

            // Read in input
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (imgui_eats_input(&event)) continue;
                if (event.type == SDL_QUIT) {
                    game_state.running = false;
                }
                if (event.type == SDL_WINDOWEVENT) {
                    if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                        game_state.running = false;
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        game_state.renderer.width = event.window.data1;
                        game_state.renderer.height = event.window.data2;
                        game_state.resized_window = true;
                    }
                }
                if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                    SDL_KeyboardEvent key = event.key;
                    if (key.repeat) continue;
                    GameInput::Button button = key.keysym.sym;
                    bool down = key.state == SDL_PRESSED;
                    if (down && global_input.eaten_by_rebind(button)) continue;
                    global_input.update_press(button, down);
                }
                if (event.type == SDL_MOUSEMOTION) {
                    if (!game_state.input.mouse_capture) continue;
                    SDL_MouseMotionEvent mouse = event.motion;
                    // TODO(ed): This mousemovement is wrong for lower refreshrate screens,
                    // it needs to be fixed somehow.
                    global_input.mouse_move = global_input.mouse_move + Vec2(mouse.xrel, mouse.yrel);
                    global_input.mouse_pos = Vec2(mouse.x, mouse.y);
                }
            }

            for (u32 i = 0; i < (u32)Ac::NUM_ACTIONS; i++) {
                game_state.input.last_frame[i] = game_state.input.current_frame[i];
                game_state.input.current_frame[i] = global_input.state[i];
            }

            game_state.input.mouse_move = global_input.mouse_move;
            game_state.input.mouse_pos = global_input.mouse_pos;

            game_state = game_lib.update(&game_state, { .update = true });
            SDL_UnlockMutex(game_state.m_event_queue);
        }

        imgui_start_frame();

        game_state = game_lib.update(&game_state, { .draw = true, .send = true });
        SDL_UnlockMutex(game_state.m_event_queue);
        SDL_SetRelativeMouseMode((SDL_bool)game_state.input.mouse_capture);

        imgui_end_frame();
        SDL_GL_SwapWindow(game_state.window);
    }
    game_lib.shutdown(&game_state);
    return 0;
}
#endif

//
// Tests are moved to a different file.
//
#include "platform_tests.h"
