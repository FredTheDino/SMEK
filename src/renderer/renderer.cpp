#include "../game.h"
#include "../util/util.h"
#include "imgui/imgui.h"
#include "opengl.h"
#include "renderer.h"

namespace GFX {

Camera Camera::init(f32 fov) {
    Camera cam = {};
    cam.set_fov(fov);
    cam.set_aspect_ratio();
    cam.up = Vec3(0, 1, 0);
    cam.rotation = H::from(Vec3(1.0, 0.0, 0.0), 0.0);
    cam.position = Vec3(0, 0, 0);
    return cam;
}

void Camera::set_fov(f32 fov) {
    dirty_perspective = true;
    this->fov = fov;
}

void Camera::set_aspect_ratio() {
    set_aspect_ratio(f32(GAMESTATE()->renderer.width) / f32(GAMESTATE()->renderer.height));
}

void Camera::set_aspect_ratio(f32 aspect_ratio) {
    dirty_perspective = true;
    this->aspect_ratio = aspect_ratio;
}

void Camera::turn(f32 jaw, f32 pitch) {
    rotation = normalized(H::from(0.0, -pitch, 0.0) * rotation * H::from(-jaw, 0.0, 0.0));
}

void Camera::move(Vec3 movement) {
    position = position + movement;
}

void Camera::move_relative(Vec3 movement) {
    Vec3 relative_move = rotation * movement;
    position = position + relative_move;
}

Vec3 Camera::get_forward() {
    return Mat::from(rotation) * Vec3(0, 0, -1);
}

void Camera::debug_draw() {
    if (current_camera() == this) {
        return;
    }
    Color4 color = Color4(0.9, 0.9, 0.5, 1.0);

    f32 size = 0.01;
    f32 inner_radius = 0.1;
    f32 outer_radius = 0.3;

    Vec3 forward = get_forward();

    push_line(position, position + forward * 0.5, Color4(), color, size * 2);
    push_circle(position, forward, inner_radius, color, 0.01);
    push_circle(position + forward * 0.2, forward, outer_radius, color, 0.01);
}

template <>
void Camera::upload(const MasterShader &shader) {
    if (dirty_perspective) {
        perspective = Mat::perspective(fov, aspect_ratio, 0.01, 100.0);
        dirty_perspective = false;
    }
    shader.upload_proj(perspective);
    Mat view = (Mat::translate(position) * Mat::from(rotation)).invert();
    shader.upload_view(view);
}

template <>
void Camera::upload(const DebugShader &shader) {
    shader.upload_proj(perspective);
    Mat view = (Mat::translate(position) * Mat::from(rotation)).invert();
    shader.upload_view(view);
}

RenderTexture RenderTexture::create(int width, int height) {
    RenderTexture t = {};
    t.width = width;
    t.height = height;
    glGenFramebuffers(1, &t.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, t.fbo);

    {
        glGenTextures(1, &t.color);
        glBindTexture(GL_TEXTURE_2D, t.color);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t.color, 0);
    }

    {
        glGenTextures(1, &t.depth_output);
        glBindTexture(GL_TEXTURE_2D, t.depth_output);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t.depth_output, 0);
    }

    {
        glGenRenderbuffers(1, &t.depth);
        glBindRenderbuffer(GL_RENDERBUFFER, t.depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, t.depth);
    }

    // Assumes both depth and color
    GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(LEN(draw_buffers), draw_buffers);

    static bool run_once = false;
    if (!run_once) {
        run_once = true;
    }

    ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
    return t;
}

void RenderTexture::destroy() {
    if (width) {
        width = 0;
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &color);
        glDeleteTextures(1, &depth_output);
        glDeleteRenderbuffers(1, &depth);
    }
}

void RenderTexture::use() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}

const Color4 color_list[] = {
#if 0
    Color4(1.0, 0.0, 1.0, 1.0),
    Color4(115, 63, 155, 255) / 255.0,
    Color4(68, 73, 163, 255) / 255.0,
    Color4(61, 120, 157, 255) / 255.0,
    Color4(51, 102, 97, 255) / 255.0,
    Color4(155, 126, 63, 255) / 255.0,
    Color4(163, 74, 68, 255) / 255.0,
    Color4(158, 61, 109, 255) / 255.0,
    Color4(102, 51, 101, 255) / 255.0,
#else
    Color4(0.0, 0.0, 0.0, 1.0),
    Color4(1.0, 0.0, 0.0, 1.0),
    Color4(0.0, 1.0, 0.0, 1.0),
    Color4(0.0, 0.0, 1.0, 1.0),
    Color4(1.0, 1.0, 0.0, 1.0),
    Color4(0.0, 1.0, 1.0, 1.0),
    Color4(1.0, 0.0, 1.0, 1.0),
    Color4(1.0, 0.0, 1.0, 1.0),
#endif
};

Color4 color(u32 index) {
    return color_list[index % LEN(color_list)];
}

void push_point(Vec3 a, Color4 c, f32 width) {
    Vec3 z = normalized(current_camera()->position - a);
    Vec3 x = Vec3(0.0, 1.0, 0.0);
    Vec3 y = cross(x, z);
    if (length_squared(y) == 0) {
        x = Vec3(1.0, 0.0, 0.0);
        y = cross(x, z);
    }
    x = cross(z, y);

    Vec3 p1, p2, p3, p4;
    p1 = a + x * width + y * width;
    p2 = a - x * width + y * width;
    p3 = a - x * width - y * width;
    p4 = a + x * width - y * width;
    push_debug_triangle(p1, c, p2, c, p3, c);
    push_debug_triangle(p1, c, p3, c, p4, c);
}

void push_line(Vec3 a, Vec3 b, Color4 color, f32 width) {
    push_line(a, b, color, color, width);
}

void push_line(Vec3 a, Vec3 b, Color4 a_color, Color4 b_color, f32 width) {
    Vec3 z = current_camera()->position - a;
    Vec3 x = a - b;
    Vec3 y = normalized(cross(x, z));
    Vec3 sideways = y * width;

    Vec3 p1, p2, p3, p4;
    p1 = a + sideways;
    p2 = a - sideways;
    p3 = b + sideways;
    p4 = b - sideways;
    push_debug_triangle(p1, a_color, p2, a_color, p3, b_color);
    push_debug_triangle(p2, a_color, p3, b_color, p4, b_color);
}

void push_circle(Vec3 center,
                 Vec3 normal,
                 f32 scale,
                 Color4 c,
                 f32 line_size,
                 u32 segments) {
    Vec3 z = normalized(normal);
    Vec3 s = cross(Vec3(0, 1, 0), z);
    if (length_squared(s) < 0.1) {
        s = cross(Vec3(1, 0, 0), z);
    }
    Vec3 t = cross(z, s);

    const f32 step = 2 * PI / segments;
    Vec3 last_offset = s * scale;
    for (u32 i = 1; i < segments; i++) {
        f32 a = i * step;

        Vec3 offset = (Math::cos(a) * s + Math::sin(a) * t) * scale;
        push_line(center + last_offset, center + offset, c, line_size);
        last_offset = offset;
    }
    push_line(center + last_offset, center + s * scale, c, line_size);
}

void Mesh::destroy() {
    glDeleteVertexArrays(11, &vao);
    glDeleteBuffers(1, &vbo);
}

Mesh Mesh::init(Vertex *verticies, u32 num_verticies) {
    u32 vao = 0, vbo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * num_verticies, verticies, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, texture));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glBindVertexArray(0);

    return { vao, vbo, num_verticies };
}

void Mesh::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, draw_length);
    glBindVertexArray(0);
}

void Skin::destroy() {
    glDeleteVertexArrays(11, &vao);
    glDeleteBuffers(1, &vbo);
}

Skin Skin::init(Vertex *verticies, u32 num_verticies) {
    u32 vao, vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * num_verticies, verticies, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, texture));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, normal));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, weight1));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, weight2));

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 2, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, weight3));
    glBindVertexArray(0);

    return { vao, vbo, num_verticies };
}

void Skin::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, draw_length);
    glBindVertexArray(0);
}

Mat Transform::to_matrix() {
    return Mat::translate(position) * Mat::from(rotation) * Mat::scale(scale);
}

Skeleton Skeleton::init(Bone *bones, u32 num_bones) {
    return { num_bones, bones };
}

void Skeleton::destroy() {
    delete[] bones;
    num_bones = 0;
}

#if 1
Mat Skeleton::matrix(i32 i) {
    if (i == -1) return Mat::scale(1.0);
    ASSERT(bones[i].index == i, "Invalid bone order");
    return bones[i].transform.to_matrix();
}

void Skeleton::draw() {
    // TODO(ed): This can be made better since the order 0...n will allways result
    // in correct updated animations.
    for (i32 i = 0; i < (i32)num_bones; i++) {
        Color4 color = i == 0 ? Color4(0, 0, 0, 1) : Color4(1, 1, 1, 1);
        matrix(i).gfx_dump(color);
    }
}
#endif

Animation Animation::init(i32 *times, i32 num_frames, Transform *trans, i32 trans_per_frame) {
    defer { delete[] times; };

    Animation anim = { trans_per_frame, num_frames };
    anim.frames = new Frame[num_frames];
    for (i32 frame = 0; frame < num_frames; frame++) {
        anim.frames[frame].t = times[frame];
        anim.frames[frame].trans = trans + (frame * trans_per_frame);
        Transform *ts = anim.frames[frame].trans;
    }
    return anim;
}

void Animation::destroy() {
    delete[] frames[0].trans;
    delete[] frames;
}

AnimatedMesh AnimatedMesh::init(AssetID skin, AssetID skeleton, AssetID animation) {
    // Type checking at runtime.
    Asset::fetch_skin(skin);
    Asset::fetch_skeleton(skeleton);
    Asset::fetch_animation(animation);

    return { skin, skeleton, animation, AnimatedMesh::STANDARD_FRAME_PER_SECOND };
}

static Mat lerp_to_matrix(Transform a, Transform b, f32 blend) {
#define LERP(a, b, l) ((a) + (l) * ((b) - (a)))
    // TODO(ed): Maybe break this out so you can lerp to a new transform.
    // If this code needs to be reused that would be great.
    return Mat::translate(LERP(a.position, b.position, blend)) *
           // NOTE(ed): This should be a slerp to be "accurate", but this is less work
           // and will probably not be noticed.
           Mat::from(lerp(a.rotation, b.rotation, blend)) * Mat::scale(LERP(a.scale, b.scale, blend));
#undef LERP
}

void AnimatedMesh::lerp_bones_to_matrix(Transform *as, Transform *bs, Mat *out, f32 blend, i32 num_bones) {
    Skeleton *skel = Asset::fetch_skeleton(skeleton);
    // Calculate the transform to the new pose.
    for (i32 i = 0; i < num_bones; i++) {
        Mat to_pose = lerp_to_matrix(as[i], bs[i], blend);

        Bone *bone = skel->bones + i;
        ASSERT(bone->index == i, "Invalid skeleton.");
        ASSERT(i > bone->parent || bone->parent == -1,
               "Invalid bone ordering, parent should always be before child.");
        if (i != 0) {
            out[i] = out[bone->parent] * to_pose;
        } else {
            out[i] = to_pose;
        }
    }

    // Calculate the transform to the new pose.
    for (i32 i = 0; i < num_bones; i++) {
        out[i] = out[i] * skel->bones[i].transform.to_matrix().invert();
    }
}

void AnimatedMesh::draw_at(float time) {
    float frame = time * seconds_to_frame;
    Animation *anim = Asset::fetch_animation(animation);
    // TODO(ed): This could be a method.
    f32 blend = 1.0;
    Animation::Frame *a = nullptr, *b = nullptr;
    for (i32 i = 0; i < anim->num_frames - 1; i++) {
        a = anim->frames + i;
        b = anim->frames + i + 1;
        if (a->t <= frame && frame <= b->t) {
            blend = (frame - a->t) / (b->t - a->t);
            break;
        }
    }
    ASSERT(a && b, "No frames given.");

    Mat *pose_mat = new Mat[anim->trans_per_frame];
    defer { delete[] pose_mat; };
    lerp_bones_to_matrix(a->trans, b->trans, pose_mat, blend, anim->trans_per_frame);

    Skeleton *skel = Asset::fetch_skeleton(skeleton);
    for (i32 i = 0; i < anim->trans_per_frame; i++) {
        (pose_mat[i] * skel->matrix(i)).gfx_dump(color(i));
    }

    master_shader().use();

    master_shader().upload_sun_dir(lighting()->sun_direction);
    master_shader().upload_sun_color(lighting()->sun_color);
    master_shader().upload_ambient_color(lighting()->ambient_color);

    master_shader().upload_bones(anim->trans_per_frame, pose_mat);
    current_camera()->upload(master_shader());
    Mat m = Mat::scale(1);
    master_shader().upload_model(m);
    Asset::fetch_skin(skin)->draw();
}

void Shader::use() { glUseProgram(program_id); }

#define FETCH_SHADER_PROP(name) \
    shader.loc_##name = glGetUniformLocation(shader.program_id, #name)

#define FETCH_SHADER_PROP_FOR_LIST(name)                                \
    shader.loc_##name = glGetUniformLocation(shader.program_id, #name); \
    shader.loc_num_##name = glGetUniformLocation(shader.program_id, "num_" #name)

MasterShader MasterShader::init() {

    MasterShader shader;
    shader.program_id = Asset::fetch_shader("MASTER_SHADER")->program_id;

    FETCH_SHADER_PROP(t);
    FETCH_SHADER_PROP(sun_color);
    FETCH_SHADER_PROP(sun_dir);
    FETCH_SHADER_PROP(ambient_color);
    FETCH_SHADER_PROP(proj);
    FETCH_SHADER_PROP(view);
    FETCH_SHADER_PROP(model);
    FETCH_SHADER_PROP(model_norm);
    FETCH_SHADER_PROP(tex);

    FETCH_SHADER_PROP_FOR_LIST(bones);

    shader.loc_pos_lights = glGetUniformLocation(shader.program_id, "light_positions");
    shader.loc_col_lights = glGetUniformLocation(shader.program_id, "light_colors");

    return shader;
}

#define F32_SHADER_PROP(classname, name) \
    void classname::upload_##name(f32 f) const { glUniform1f(loc_##name, f); }

#define U32_SHADER_PROP(classname, name) \
    void classname::upload_##name(u32 f) const { glUniform1i(loc_##name, f); }

#define U32_SHADER_PROP(classname, name) \
    void classname::upload_##name(u32 f) const { glUniform1i(loc_##name, f); }

#define V3_SHADER_PROP(classname, name) \
    void classname::upload_##name(Vec3 f) const { glUniform3f(loc_##name, f.x, f.y, f.z); }

#define V4_SHADER_PROP(classname, name) \
    void classname::upload_##name(Vec4 f) const { glUniform4f(loc_##name, f.x, f.y, f.z, f.w); }

#define MAT_SHADER_PROP(classname, name) \
    void classname::upload_##name(Mat &m) const { glUniformMatrix4fv(loc_##name, 1, true, m.data()); }

#define MATS_SHADER_PROP(classname, name)                     \
    void classname::upload_##name(u32 num, Mat *m) const {    \
        glUniform1i(loc_num_##name, num);                     \
        glUniformMatrix4fv(loc_##name, num, true, m->data()); \
    }

F32_SHADER_PROP(MasterShader, t);
U32_SHADER_PROP(MasterShader, tex);

MAT_SHADER_PROP(MasterShader, proj);
MAT_SHADER_PROP(MasterShader, view);
MAT_SHADER_PROP(MasterShader, model);
MAT_SHADER_PROP(MasterShader, model_norm);

V3_SHADER_PROP(MasterShader, sun_dir);
V3_SHADER_PROP(MasterShader, sun_color);
V3_SHADER_PROP(MasterShader, ambient_color);

MATS_SHADER_PROP(MasterShader, bones);

void MasterShader::upload_lights(Vec3 *positions, Color3 *colors) const {
    glUniform3fv(loc_col_lights, MAX_LIGHTS, colors->_);
    glUniform3fv(loc_pos_lights, MAX_LIGHTS, positions->_);
}

PostProcessShader post_process_shader() {
    if (Asset::needs_reload("POSTPROCESS_SHADER"))
        GAMESTATE()->renderer.post_process_shader = PostProcessShader::init();
    return GAMESTATE()->renderer.post_process_shader;
}

MasterShader master_shader() {
    if (Asset::needs_reload("MASTER_SHADER"))
        GAMESTATE()->renderer.master_shader = MasterShader::init();
    return GAMESTATE()->renderer.master_shader;
}

DebugShader debug_shader() {
    if (Asset::needs_reload("DEBUG_SHADER"))
        GAMESTATE()->renderer.debug_shader = DebugShader::init();
    return GAMESTATE()->renderer.debug_shader;
}

Camera *current_camera() {
    if (GAMESTATE()->renderer.use_debug_cam)
        return &GAMESTATE()->renderer.debug_camera;
    else
        return &GAMESTATE()->renderer.gameplay_camera;
}

Camera *debug_camera() {
    return &GAMESTATE()->renderer.debug_camera;
}

Camera *gameplay_camera() {
    return &GAMESTATE()->renderer.gameplay_camera;
}

DebugShader DebugShader::init() {
    DebugShader shader;
    shader.program_id = Asset::fetch_shader("DEBUG_SHADER")->program_id;

    FETCH_SHADER_PROP(proj);
    FETCH_SHADER_PROP(view);
    FETCH_SHADER_PROP(model);

    return shader;
}

MAT_SHADER_PROP(DebugShader, proj);
MAT_SHADER_PROP(DebugShader, view);
MAT_SHADER_PROP(DebugShader, model);

PostProcessShader PostProcessShader::init() {
    PostProcessShader shader;
    shader.program_id = Asset::fetch_shader("POSTPROCESS_SHADER")->program_id;

    FETCH_SHADER_PROP(t);
    FETCH_SHADER_PROP(tex);

    return shader;
}

F32_SHADER_PROP(PostProcessShader, t);
U32_SHADER_PROP(PostProcessShader, tex);

void Shader::destroy() {
    glDeleteProgram(program_id);
}

Shader Shader::compile(const char *asset, const char *source) {
    auto shader_error_check = [asset](u32 shader) -> bool {
        i32 success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
            ERR("Shader error: ({}) {}", asset, info_log);
        }
        return success;
    };

    auto program_error_check = [asset](u32 program) -> bool {
        i32 success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
            ERR("Program error: ({}) {}", asset, info_log);
        }
        return success;
    };

    // Vertex
    const char *vertex_source[] = {
        "#version 330\n"
        "#define VERT\n",
        source
    };

    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    defer { glDeleteShader(vertex_shader); };

    glShaderSource(vertex_shader, sizeof(vertex_source) / sizeof(vertex_source[0]), vertex_source, NULL);
    glCompileShader(vertex_shader);

    if (!shader_error_check(vertex_shader)) return { -1 };

    // Fragment
    const char *fragment_source[] = {
        "#version 330\n"
        "#define FRAG\n",
        source
    };

    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    defer { glDeleteShader(fragment_shader); };
    glShaderSource(fragment_shader, sizeof(fragment_source) / sizeof(fragment_source[0]), fragment_source, NULL);
    glCompileShader(fragment_shader);

    if (!shader_error_check(fragment_shader)) return { -1 };

    i32 program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    if (!program_error_check(program)) {
        glDeleteProgram(program);
        return { -1 };
    }

    return { program };
}

Texture Texture::upload(u32 width, u32 height, u32 components, u8 *data, Sampling sampling) {
    u32 texture = 0;
#ifndef TESTS
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum format = GL_RED;
    if (components == 1) format = GL_RED;
    else if (components == 2)
        format = GL_RG;
    else if (components == 3)
        format = GL_RGB;
    else if (components == 4)
        format = GL_RGBA;
    else
        UNREACHABLE("Invalid number of components ({})", components);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    GLenum gl_sampling = GL_LINEAR;
    if (sampling == Sampling::LINEAR) gl_sampling = GL_LINEAR;
    else if (sampling == Sampling::NEAREST)
        gl_sampling = GL_NEAREST;
    else
        UNREACHABLE("Unsupported sampling ({})", components);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_sampling);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_sampling);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
    return { .texture_id = texture, .width = width, .height = height, .components = components };
}

void Texture::bind(u32 texture_slot) {
    ASSERT_LT(texture_slot, 80);
    glActiveTexture(GL_TEXTURE0 + texture_slot);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glActiveTexture(GL_TEXTURE0 + 79); // Hardcoded since it's the "minimum maximum".
}

void Texture::destroy() {
    glDeleteTextures(1, &texture_id);
}

bool init(GameState *gs, i32 width, i32 height) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    gs->renderer.width = width;
    gs->renderer.height = height;

    int window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
    if (gs->allow_user_resize_window) {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    gs->window = SDL_CreateWindow("SMEK - The new begining",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  width,
                                  height,
                                  window_flags);

    if (gs->window == NULL) {
        ERR("Failed to create OpenGL window \"{}\"", SDL_GetError());
        return false;
    }

    gs->gl_context = SDL_GL_CreateContext(gs->window);
    SDL_GL_MakeCurrent(gs->window, gs->gl_context);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        ERR("Failed to load OpenGL function.");
        return false;
    }

    Mesh::Vertex a, b, c, d;
    a = { { -1., -1., +0. }, { 0., 0. }, {} };
    b = { { +1., -1., +0. }, { 1., 0. }, {} };
    c = { { +1., +1., +0. }, { 1., 1. }, {} };
    d = { { -1., +1., +0. }, { 0., 1. }, {} };

    Mesh::Vertex verticies[] = { a, b, c, a, c, d };
    gs->renderer.quad = Mesh::init(verticies, LEN(verticies));

    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_ShowWindow(gs->window);
    SDL_GL_SwapWindow(gs->window);

    gs->renderer.master_shader = MasterShader::init();
    gs->renderer.debug_shader = DebugShader::init();

    gs->renderer.primitives.push_back(DebugPrimitive::init());
    return true;
}

RenderTexture render_target() {
    return GAMESTATE()->renderer.target;
}

void remake_render_target() {
    GAMESTATE()->renderer.target.destroy();
    GAMESTATE()->renderer.target = GFX::RenderTexture::create(GAMESTATE()->renderer.width,
                                                              GAMESTATE()->renderer.height);
}

void Renderer::set_screen_resolution(i32 new_width, i32 new_height) {
    width = new_width;
    height = new_height;

    SDL_SetWindowSize(GAMESTATE()->window, width, height);
    remake_render_target();

    debug_camera.set_aspect_ratio();
    gameplay_camera.set_aspect_ratio();
}

void set_screen_resolution() {
    i32 width = GAMESTATE()->renderer.width;
    i32 height = GAMESTATE()->renderer.height;
    GAMESTATE()->renderer.set_screen_resolution(width, height);
}

void set_screen_resolution(i32 width, i32 height) {
    GAMESTATE()->renderer.set_screen_resolution(width, height);
}

void push_mesh(AssetID mesh, AssetID texture, Vec3 position, Quat rotation, Vec3 scale) {
    MasterShader shader = master_shader();
    Asset::fetch_texture(texture)->bind(1);
    shader.upload_tex(1);

    Mat model = Mat::translate(position) * Mat::from(rotation) * Mat::scale(scale);
    shader.upload_model(model);
    Mat model_norm = model.invert().transpose();
    model_norm.gfx_dump();
    shader.upload_model_norm(model_norm);
    Asset::fetch_mesh(mesh)->draw();
}

void set_camera_mode(bool debug_mode) {
    GAMESTATE()->renderer.use_debug_cam = debug_mode;
}

Lighting *lighting() {
    return &GAMESTATE()->renderer.lighting;
}

bool reload(GameState *gs) {
    SDL_GL_MakeCurrent(gs->window, gs->gl_context);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        ERR("Failed to reload OpenGL function.");
        return false;
    }

    return true;
}

void deinit(GameState *gs) {
    SDL_DestroyWindow(gs->window);
    SDL_GL_DeleteContext(gs->gl_context);

    SDL_Quit();
}

DebugPrimitive DebugPrimitive::init() {
    u32 vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    static u32 calls = 0;
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugPrimitive::Vertex) * VERTS_PER_BUFFER, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, color));
    glBindVertexArray(0);

    DebugPrimitive d = {};
    d.vao = vao;
    d.vbo = vbo;
    d.buffer = new Vertex[VERTS_PER_BUFFER];
    return d;
}

bool DebugPrimitive::push_triangle(Vertex v1, Vertex v2, Vertex v3) {
    if (size + 3 >= VERTS_PER_BUFFER) return false;
    buffer[size++] = v1;
    buffer[size++] = v2;
    buffer[size++] = v3;
    return true;
}

void DebugPrimitive::draw() {
    if (size == 0) return;
    ASSERT(size % 3 == 0, "Reached invalid state for this debug primitive");

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(Vertex), buffer);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, color));

    glDrawArrays(GL_TRIANGLES, 0, size);

    // glPointSize(10.0);
    // glDrawArrays(GL_POINTS, 0, size);
    glBindVertexArray(0);
}

void DebugPrimitive::clear() {
    size = 0;
}

void push_debug_triangle(Vec3 p1, Color4 c1, Vec3 p2, Color4 c2, Vec3 p3, Color4 c3) {
    Renderer *renderer = &GAMESTATE()->renderer;
    while (!renderer->primitives[renderer->first_empty].push_triangle({ p1, c1 }, { p2, c2 }, { p3, c3 })) {
        if (++renderer->first_empty == renderer->primitives.size())
            renderer->primitives.push_back(DebugPrimitive::init());
    }
}

void draw_primitivs() {
    current_camera()->upload(debug_shader());
    std::vector<DebugPrimitive> *primitives = &GAMESTATE()->renderer.primitives;
    for (DebugPrimitive &p : *primitives) {
        p.draw();
        p.clear();
    }
    GAMESTATE()->renderer.first_empty = 0;
}

void resolve_to_screen(RenderTexture texture) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, GAMESTATE()->renderer.width, GAMESTATE()->renderer.height);

    glClearColor(0.1, 0.1, 0.1, 1); // We don't need to do this...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    post_process_shader().use();
    post_process_shader().upload_t(time());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.color);
    post_process_shader().upload_tex(0);

    GAMESTATE()->renderer.quad.draw();
}

} // namespace GFX
