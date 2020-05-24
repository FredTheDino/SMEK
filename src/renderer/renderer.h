#pragma once
#include "../math/smek_vec.h"

#include "../asset/asset.h"

#include <vector>

struct GameState;

///# Renderer
// The renderer is the code that draws graphics to the
// screen.

namespace GFX {

///* Mesh
struct Mesh;

///* Shader
struct Shader;

///* Texture
struct Texture;

///* Renderer
// The collection of the entire rendering state.
struct Renderer;

struct Mesh {
    u32 vao, vbo;
    u32 draw_length;

    static Mesh init(Asset::Model *model);

    void draw();
};

struct Shader {
    i32 program_id;

    void use();

    bool is_valid() { return program_id != -1; }

    static Shader compile(const char *source);
};

struct Texture {
    u32 texture_id;

    enum class Sampling {
        LINEAR,
        NEAREST,
    };

    void bind(u32 texture_slot=0);

    static Texture upload(u32 width, u32 height, u32 components, u8 *data, Sampling sampling);
    static Texture upload(Asset::Image *image, Sampling sampling);
};

Shader default_shader();

struct Renderer {
    Shader shader;
};

///*
// Initalize the graphics pipeline.
bool init(GameState *gs, const char *shader_source, int width=680, int height=480);

///*
// Reloads opengl function pointers and such, which is faster
// to setup-again than to copy over.
bool reload(GameState *gs);

///*
// Destroy the graphics pipeline.
void deinit(GameState *gs);

} // namespace GFX
