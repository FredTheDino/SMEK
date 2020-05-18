#pragma once
#include "../math/smek_vec.h"

#include <vector>

namespace GFX {

struct Mesh {
    u32 vao, vbo;
    u32 draw_length;

    static Mesh init(std::vector<Vec3> points);

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
};

Shader default_shader();

///*
// Initalize the graphics pipeline.
bool init(const char *shader_source);

///*
// Destroy the graphics pipeline.
void deinit();

} // namespace GFX
