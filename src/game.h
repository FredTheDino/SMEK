#pragma once
#include "SDL2/SDL.h"

#include "asset/asset.h"
#include "renderer/renderer.h"
#include "input.h"
#include "audio.h"
#include "entity/entity.h"

#include <queue>

#include "event.h"

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
    std::queue<EventSystem::Event> event_queue;

    SDL_threadID main_thread;
    f32 delta;
    f32 time;
    u32 frame;

    f32 player_jump_speed = 2.0;
    f32 player_movement_speed = 4.0;
    f32 player_mouse_sensitivity = 1.0;

    bool running;

#ifndef IMGUI_DISABLE
    // state for imgui
    bool show_create_sound_window = false;
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
typedef void(*GameInitFunc)(GameState *, int, int);

enum class GameStateUpdateMode {
    RENDER,
    UPDATE,
    UPDATE_AND_RENDER,
};
using GSUM = GameStateUpdateMode;

f32 delta();
f32 time();
u32 frame();

bool should_update(GSUM gsmu);
bool should_draw(GSUM gsmu);

///*
// Reloads eventual global state. If you can get away with storing
// things on the global game state you should, but this is useful
// for certain callbacks and opengl function pointers.
extern "C" void reload_game(GameState *gamestate);
typedef void(*GameReloadFunc)(GameState *);

///*
// Steps the game one frame forward. Doesn't have side-effects
// outside of the GameState object, but the new one is returned.
extern "C" GameState update_game(GameState *gamestate, GSUM mode=GSUM::UPDATE_AND_RENDER);
typedef GameState(*GameUpdateFunc)(GameState *, GSUM);
