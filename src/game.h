#pragma once
#include "SDL2/SDL.h"

#include "asset/asset.h"

///*
struct GameState {
    SDL_Window *window;
    SDL_GLContext gl_context;
    float time;
    Asset::System asset_system = {};
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
void init_game(GameState *gamestate);
typedef void(*GameInitFunc)(GameState *);

enum class GameStateUpdateMode {
    RENDER,
    UPDATE,
    UPDATE_AND_RENDER,
};
using GSUM = GameStateUpdateMode;

///*
// Steps the game one frame forward. Doesn't have side-effects
// outside of the GameState object, but the new one is returned.
GameState update_game(float time, GameState *gamestate, GSUM mode=GSUM::UPDATE_AND_RENDER);
typedef GameState(*GameUpdateFunc)(float time, GameState *, GSUM);

