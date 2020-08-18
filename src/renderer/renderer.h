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
    i32 parent;
    i32 index;
    union {
        struct {
            Vec3 scale;
            Quat rotation;
            Vec3 position;
        } t;
        Transform transform;
    };

    Bone() {}
};

struct Skeleton {
    u32 num_bones;
    Bone *bones;

    static Skeleton init(Bone *bones, u32 num_bones);

    void destroy();

    Mat matrix(i32 i);
    void draw();
};

struct Animation {
    struct Frame {
        i32 t;
        Transform *trans;
    };
    i32 trans_per_frame;
    i32 num_frames;

    Frame *frames;

    static Animation init(i32 *times, i32 num_frames, Transform *trans, i32 trans_per_frame);

    const Frame &operator[](i32 i) {
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

    void lerp_bones_to_matrix(Transform *as, Transform *bs, Mat *out, f32 blend, i32 num_bones);
    void draw_at(float time);

    static AnimatedMesh init(AssetID skin, AssetID skeleton, AssetID animation);
};

struct Shader {
    i32 program_id;

    void use();

    void destroy();

    bool is_valid() { return program_id != -1; }

    static Shader compile(const char *asset, const char *source);
};

#define F32_SHADER_PROP(name)\
    u32 loc_ ##name;\
    void upload_ ##name (f32) const;

#define U32_SHADER_PROP(name)\
    u32 loc_ ##name;\
    void upload_ ##name (u32) const;

#define V3_SHADER_PROP(name)\
    u32 loc_ ##name;\
    void upload_ ##name (Vec3) const;

#define V4_SHADER_PROP(name)\
    u32 loc_ ##name;\
    void upload_ ##name (Vec4) const;

#define MAT_SHADER_PROP(name)\
    u32 loc_ ##name;\
    void upload_ ##name (Mat &) const;

#define MATS_SHADER_PROP(name)\
    u32 loc_num_ ##name;\
    u32 loc_ ##name;\
    void upload_ ##name (u32, Mat *) const;

struct MasterShader: public Shader {
    F32_SHADER_PROP(t);
    U32_SHADER_PROP(tex);

    MAT_SHADER_PROP(proj);
    MAT_SHADER_PROP(view);
    MAT_SHADER_PROP(model);

    V3_SHADER_PROP(sun_dir);
    V3_SHADER_PROP(sun_color);
    V3_SHADER_PROP(ambient_color);

    // Light prop, since we should only have one.
    u32 loc_pos_lights;
    u32 loc_col_lights;
    void upload_lights(Vec3 *, Vec3 *) const;

    MATS_SHADER_PROP(bones);

    static MasterShader init();
};

struct PostProcessShader: public Shader {
    F32_SHADER_PROP(t);
    U32_SHADER_PROP(tex);

    static PostProcessShader init();
};

struct DebugShader: public Shader {
    MAT_SHADER_PROP(proj);
    MAT_SHADER_PROP(view);
    MAT_SHADER_PROP(model);

    static DebugShader init();
};


#undef F32_SHADER_PROP
#undef U32_SHADER_PROP
#undef V3_SHADER_PROP
#undef V4_SHADER_PROP
#undef MAT_SHADER_PROP
#undef MATS_SHADER_PROP

struct Texture {
    u32 texture_id;

    u32 width;
    u32 height;
    u32 components;


    enum class Sampling {
        LINEAR,
        NEAREST,
    };

    void bind(u32 texture_slot=0);
    void destroy();

    static Texture upload(u32 width, u32 height, u32 components, u8 *data, Sampling sampling);
};

struct RenderTexture {
    i32 width, height;
    u32 fbo;
    u32 color;
    u32 depth_output;
    u32 depth;

    void use();
    void destroy();

    static RenderTexture create(int width, int height, bool use_depth, bool use_color);
};

///*
// Fetches the master shader.
MasterShader master_shader();

///*
// Fetches the debug shader.
PostProcessShader post_process_shader();

///*
// Fetches the debug shader.
DebugShader debug_shader();

///*
// Fetches the master shader.
Camera *current_camera();

///*
// Fetches the debug camera
Camera *debug_camera();

///*
// Fetches the player camera
Camera *gameplay_camera();

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

// NOTE(ed): Should match "MAX_LIGHTS" in master_shader.glsl
const u32 MAX_LIGHTS = 10;

struct Lighting {
    Vec3 sun_direction;
    Vec3 sun_color;

    Vec3 ambient_color;

    Vec3 light_positions[MAX_LIGHTS];
    // Has to be initalized to zero to mark as not being used.
    Vec3 light_colors[MAX_LIGHTS] = {};
};

struct Renderer {
    u32 width;
    u32 height;

    Mesh quad;
    Lighting lighting;

    bool use_debug_cam;
    Camera debug_camera;
    Camera gameplay_camera;

    MasterShader master_shader;
    PostProcessShader post_process_shader;
    DebugShader debug_shader;

    u32 first_empty;
    std::vector<DebugPrimitive> primitives;
};

///*
// Initalize the graphics pipeline.
bool init(GameState *gs, i32 width=680, i32 height=480);

///*
// Returns the lighting struct.
Lighting *lighting();

///*
// Sets the camera to be used when rendering. There's a "debug"
// camera and a "gameplay" camera. Passing true uses the "debug"
// camera.
void set_camera_mode(bool debug_mode=false);

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

///*
// Renders the debug primitivs to the screen.
void draw_primitivs();

///*
// Applies the post processing shader and renders
// the texture to the screen.
void resolve_to_screen(RenderTexture texture);

template<> void Camera::upload<MasterShader>(const MasterShader &shader);
template<> void Camera::upload<DebugShader>(const DebugShader &shader);


} // namespace GFX
