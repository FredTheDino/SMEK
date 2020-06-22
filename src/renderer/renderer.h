#pragma once
#include "../math/smek_vec.h"
#include "../math/smek_mat4.h"

#include <vector>

struct GameState;

///# Renderer
// The renderer is the code that draws graphics to the
// screen.

namespace Asset {
    struct Model;
};

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

    f32 aspect_ratio;
    f32 fov;
    bool dirty_perspective;

    // NOTE(ed): Given in radians.
    static Camera init(f32 fov=PI / 4);

    // NOTE(ed): Given in radians.
    void set_fov(f32 fov);

    void set_aspect_ratio(f32 aspect_ratio);

    void turn(f32 jaw, f32 pitch);

    void move(Vec3 movement);
    void move_relative(Vec3 movement);

    template<typename S>
    void upload(const S &s);
};


struct Mesh {
    struct Vertex {
        Vec3 position;
        Vec2 texture;
        Vec3 normal;
    };

    u32 vao, vbo;
    u32 draw_length;

    static Mesh init(Vertex *vericies, u32 num_verticies);

    void destroy();

    void draw();
};

struct Skin {
    struct Vertex {
        Vec3 position;
        Vec2 texture;
        Vec3 normal;
        Vec2 weight1;
        Vec2 weight2;
        Vec2 weight3;
    };

    u32 vao, vbo;
    u32 draw_length;

    static Skin init(Vertex *vericies, u32 num_verticies);

    void destroy();

    void draw();
};

struct Transform {
    Vec3 scale;
    Quat rotation;
    Vec3 position;

    Mat to_matrix();
};

struct Bone {
    int parent;
    int index;
    union {
        struct {
            Vec3 scale;
            Quat rotation;
            Vec3 position;
        };
        Transform transform;
    };

    Bone() {}
};

struct Skeleton {
    u32 num_bones;
    Bone *bones;

    static Skeleton init(Bone *bones, u32 num_bones);

    void destroy();

    Mat matrix(int i);
    void draw();
};

struct Animation {
    struct Frame {
        u32 t;
        Transform *trans;
    };
    u32 trans_per_frame;
    u32 num_frames;

    Frame *frames;

    static Animation init(u32 *times, u32 num_frames, Transform *trans, u32 trans_per_frame);

    const Frame &operator[](int i) {
        return frames[i];
    }

    void destroy();
};

struct AnimatedMesh {
    static constexpr float STANDARD_FRAME_PER_SECOND = 1.0 / 60.0;
    AssetID skin;
    AssetID skeleton;
    AssetID animation;

    // f32 time; // Add this in to let the animation be stepped through.
    f32 seconds_to_frame;

    void lerp_bones_to_matrix(Transform *as, Transform *bs, Mat *out, f32 blend, u32 num_bones);
    void draw_at(float time);

    static AnimatedMesh init(AssetID skin, AssetID skeleton, AssetID animation);
};

struct Shader {
    i32 program_id;

    void use();

    void destroy();

    bool is_valid() { return program_id != -1; }

    static Shader compile(const char *source);
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

struct DebugShader: public Shader {
    MAT_SHADER_PROP(proj);
    MAT_SHADER_PROP(view);
    MAT_SHADER_PROP(model);

    static DebugShader init();
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
    void destroy();

    static Texture upload(u32 width, u32 height, u32 components, u8 *data, Sampling sampling);
};

///*
// Fetches the master shader.
MasterShader master_shader();

///*
// Fetches the master shader.
DebugShader debug_shader();

///*
// Fetches the master shader.
Camera *main_camera();

struct DebugPrimitive {
    struct Vertex {
        Vec3 position;
        Vec4 color;
    };
    static const u32 VERTS_PER_BUFFER = 300;

    static DebugPrimitive init();

    void clear();

    bool push_triangle(Vertex v1, Vertex v2, Vertex v3);

    void draw();

    u32 vao, vbo;

    u32 size;
    Vertex *buffer;
};

struct Renderer {
    u32 width;
    u32 height;

    Camera main_camera;

    MasterShader master_shader;
    DebugShader debug_shader;

    u32 first_empty;
    std::vector<DebugPrimitive> primitives;
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

///*
// Returns a nice color, garanteed to match other
// colors returned from here.
Vec4 color(u32 index=0);

///*
// Draws a point in a with color c.
void push_point(Vec3 a, Vec4 c=color(), f32 width=0.1);

///*
// Draws a line form A to B, in the given color.
void push_line(Vec3 a, Vec3 b, Vec4 c=color(), f32 width=0.1);
void push_line(Vec3 a, Vec3 b, Vec4 a_color, Vec4 b_color, f32 width=0.1);

///*
// Adds one triangle to be drawn during
// the debug render call.
void push_debug_triangle(Vec3 p1, Vec4 c1, Vec3 p2, Vec4 c2, Vec3 p3, Vec4 c3);

void draw_primitivs();

template<> void Camera::upload<MasterShader>(const MasterShader &shader);
template<> void Camera::upload<DebugShader>(const DebugShader &shader);


} // namespace GFX
