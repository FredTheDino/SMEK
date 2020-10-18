#pragma once
#include "SDL.h"

#include "asset/asset.h"
#include "renderer/renderer.h"
#include "input.h"
#include "audio.h"
#include "physics/physics.h"
#include "entity/entity.h"
#include "network/network.h"

#include <queue>

#include "event.h"
#include "imgui_state.h"

///# Game
//

///* GameState
struct GameState {
    SDL_Window *window;
    SDL_GLContext gl_context;
    Asset::System asset_system;
    GFX::Renderer renderer = {};
    Input::Input input = {};
    Audio::AudioStruct *audio_struct;
    EntitySystem entity_system;
    Physics::PhysicsEngine physics_engine;
    SDL_mutex *m_event_queue;
    std::queue<Event> event_queue;
    Network network = {};

    SDL_mutex *m_reload_lib;
    bool *reload_lib;

    SDL_threadID main_thread;
    f32 delta;
    f32 time;
    u32 frame;

    f32 player_jump_speed = 2.0;
    f32 player_movement_speed = 4.0;
    f32 player_mouse_sensitivity = 1.0;

    EntityID lights[2];

    bool running;
    bool auto_scale_window;
    bool center_window;
    bool resized_window;
    bool full_screen_window;
    bool allow_user_resize_window;

#ifdef IMGUI_ENABLE
    ImGuiState imgui = {};
#endif
};

///*
// Returns the gamestate for the game this frame.
// Don't keep this pointer since it might be
// invalid on the next frame.
GameState *GAMESTATE();

///*
// Initalizes the game by setting up the renderer and
// things like that. The gamestate is expected to have
// a GLContext on it.
extern "C" void init_game(GameState *gamestate, int width, int height);
typedef void (*GameInitFunc)(GameState *, int, int);

struct GameStateUpdateMode {
    bool draw = false;
    bool update = false;
    bool send = false;
};
using GSUM = GameStateUpdateMode;

f32 delta();
f32 time();
u32 frame();

///*
// Reloads eventual global state. If you can get away with storing
// things on the global game state you should, but this is useful
// for certain callbacks and opengl function pointers.
extern "C" void reload_game(GameState *gamestate);
typedef void (*GameReloadFunc)(GameState *);

#ifdef __clang__
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif

///*
// Steps the game one frame forward. Doesn't have side-effects
// outside of the GameState object, but the new one is returned.
extern "C" GameState update_game(GameState *gamestate, GSUM mode);
typedef GameState (*GameUpdateFunc)(GameState *, GSUM);

///*
// Shutdown everything nicely.
extern "C" void shutdown_game(GameState *gamestate);
typedef void (*GameShutdownFunc)(GameState *);
