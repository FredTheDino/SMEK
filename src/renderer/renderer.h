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
};

Shader default_shader();

///*
// Initalize the graphics pipeline.
bool init();

///*
// Destroy the graphics pipeline.
void deinit();

} // namespace GFX
