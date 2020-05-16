#pragma once
#include "SDL2/SDL.h"

///*
struct GameState {
    SDL_Window *window;
    SDL_GLContext gl_context;
    float time;
};

extern GameState _global_gs;
