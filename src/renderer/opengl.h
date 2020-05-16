#pragma once

namespace GL {

constexpr u32 DEPTH_BUFFER_BIT = 0x00000100;
constexpr u32 COLOR_BUFFER_BIT = 0x00004000;

#define GL_FUNC(ret, name, ...)\
    typedef ret (* _ ## name ## _func)(__VA_ARGS__);\
    _ ## name ## _func name = nullptr;

GL_FUNC(void, ClearColor, f32, f32, f32, f32)
GL_FUNC(void, Clear, u32)
GL_FUNC(void, Blargh, u32)

#define GL_LOAD(name)\
    name = (_ ## name ## _func) SDL_GL_GetProcAddress("gl" #name);\
    if (!name) { ERROR("Failed to load OpenGL func: \"" #name "\""); success = false; }

bool load_procs() {
    bool success = true;
    GL_LOAD(ClearColor)
    GL_LOAD(Clear)
    GL_LOAD(Blargh)

    return success;
}

}
