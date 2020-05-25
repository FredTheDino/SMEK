#pragma once
#include "SDL2/SDL.h"

#include "asset/asset.h"
#include "renderer/renderer.h"
#include "input.h"
#include "audio.h"

///* GameState
struct GameState {
    SDL_Window *window;
    SDL_GLContext gl_context;
    float time;
    Asset::System asset_system = {};
    GFX::Renderer renderer = {};
    Input::Input input = {};
    Audio::AudioStruct *audio_struct;

    bool running;
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
extern "C" void init_game(GameState *gamestate);
typedef void(*GameInitFunc)(GameState *);

enum class GameStateUpdateMode {
    RENDER,
    UPDATE,
    UPDATE_AND_RENDER,
};
using GSUM = GameStateUpdateMode;

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
