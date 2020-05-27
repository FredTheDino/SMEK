#pragma once
#include "../math/smek_vec.h"
#include "../math/smek_mat4.h"

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

///* Camera
// Lets you control where the player is looking.
struct Camera;

struct Camera {
    Mat perspective;
    H rotation;
    Vec3 up;
    Vec3 position;

    // NOTE(ed): Given in radians.
    static Camera init(f32 fov=PI / 4);

    // NOTE(ed): Given in radians.
    void set_fov(f32 fov);

    void look_at(Vec3 target);
    void look_at_from(Vec3 from, Vec3 target);

    void turn(f32 jaw, f32 pitch);

    void move(Vec3 movement);
    void move_relative(Vec3 movement);

    template<typename S>
    void upload(const S &s);
};

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

    static Shader compile(AssetID source_id);
};

#define F32_SHADER_PROP(name)\
    u32 loc_ ##name;\
    void upload_ ##name (f32) const;

#define U32_SHADER_PROP(name)\
    u32 loc_ ##name;\
    void upload_ ##name (u32) const;

#define MAT_SHADER_PROP(name)\
    u32 loc_ ##name;\
    void upload_ ##name (Mat &) const;

struct MasterShader: public Shader {
    F32_SHADER_PROP(t);
    MAT_SHADER_PROP(proj);
    MAT_SHADER_PROP(view);
    MAT_SHADER_PROP(model);
    U32_SHADER_PROP(tex);

    static MasterShader init();
};

#undef F32_SHADER_PROP
#undef U32_SHADER_PROP
#undef MAT_SHADER_PROP

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

MasterShader master_shader();

struct Renderer {
    MasterShader master_shader;
};

///*
// Initalize the graphics pipeline.
bool init(GameState *gs, int width=680, int height=480);

///*
// Reloads opengl function pointers and such, which is faster
// to setup-again than to copy over.
bool reload(GameState *gs);

///*
// Destroy the graphics pipeline.
void deinit(GameState *gs);

} // namespace GFX
