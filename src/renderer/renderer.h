#pragma once
#include "opengl.h"
#include "../main.h"

namespace GFX {

bool init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        ERROR("Failed to initalize SDL \"%s\"", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    const int width = 640;
    const int height = 480;
    _global_gs.window = SDL_CreateWindow("SMEK - The new begining",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_OPENGL);

    if (_global_gs.window == NULL) {
        ERROR("Failed to create OpenGL window \"%s\"", SDL_GetError());
        return false;
    }

    _global_gs.gl_context = SDL_GL_CreateContext(_global_gs.window);

    return GL::load_procs();
}

void deinit() {
    SDL_Quit();
}

} // namespace GFX
