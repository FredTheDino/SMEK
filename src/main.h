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

GameState *global_gamestate();
