#include "../game.h"
#include "../util/util.h"
#include "opengl.h"
#include "renderer.h"

namespace GFX {

Camera Camera::init(f32 fov) {
    Camera cam = {};
    cam.set_fov(fov);
    cam.set_aspect_ratio(f32(GAMESTATE()->renderer.width) / GAMESTATE()->renderer.height);
    cam.up = Vec3(0, 1, 0);
    cam.rotation = H::from(Vec3(1.0, 0.0, 0.0), 0.0);
    cam.position = Vec3(0, 0, 0);
    return cam;
}

void Camera::set_fov(f32 fov) {
    dirty_perspective = true;
    this->fov = fov;
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

template<>
void Camera::upload(const MasterShader &shader) {
    if (dirty_perspective) {
        perspective = Mat::perspective(fov, aspect_ratio, 0.01, 10.0);
        dirty_perspective = false;
    }
    shader.upload_proj(perspective);
    Mat view = (Mat::translate(position) * Mat::from(rotation)).invert();
    shader.upload_view(view);
}

template<>
void Camera::upload(const DebugShader &shader) {
    shader.upload_proj(perspective);
    Mat view = (Mat::translate(position) * Mat::from(rotation)).invert();
    shader.upload_view(view);
}

const Vec4 color_list[] = {
    Vec4(1.0, 0.0, 1.0, 1.0),
    Vec4(115, 63, 155, 255) / 255.0,
    Vec4(68, 73, 163, 255) / 255.0,
    Vec4(61, 120, 157, 255) / 255.0,
    Vec4(51, 102, 97, 255) / 255.0,
    Vec4(155, 126, 63, 255) / 255.0,
    Vec4(163, 74, 68, 255) / 255.0,
    Vec4(158, 61, 109, 255) / 255.0,
    Vec4(102, 51, 101, 255) / 255.0,
};

Vec4 color(u32 index) {
    ASSERT(index < LEN(color_list), "Trying to fetch color that doesn't exist");
    return color_list[index];
}

void push_point(Vec3 a, Vec4 c, f32 width) {
    push_line(a - Vec3(width, width, width) * 0.5, a + Vec3(width, width, width) * 0.5, c, c, width);
}

void push_line(Vec3 a, Vec3 b, Vec4 color, f32 width) {
    push_line(a, b, color, color, width);
}

void push_line(Vec3 a, Vec3 b, Vec4 a_color, Vec4 b_color, f32 width) {
    Vec3 sideways = normalized(cross(a - b, Vec3(0.0, 1.0))) * width;
    Vec3 p1, p2, p3, p4;
    p1 = a + sideways;
    p2 = a - sideways;
    p3 = b + sideways;
    p4 = b - sideways;
    push_debug_triangle(p1, a_color, p2, a_color, p3, b_color);
    push_debug_triangle(p2, a_color, p3, b_color, p4, b_color);
}

void Mesh::destroy() {
    glDeleteVertexArrays(11, &vao);
    glDeleteBuffers(1, &vbo);
}

Mesh Mesh::init(Vertex *verticies, u32 num_verticies) {
    u32 vao, vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * num_verticies, verticies, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, texture));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, normal));
    glBindVertexArray(0);

    return {vao, vbo, num_verticies};
}

void Mesh::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, draw_length);
    glBindVertexArray(0);
}

void SkinnedMesh::destroy() {
    glDeleteVertexArrays(11, &vao);
    glDeleteBuffers(1, &vbo);
}

SkinnedMesh SkinnedMesh::init(Vertex *verticies, u32 num_verticies) {
    u32 vao, vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * num_verticies, verticies, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, texture));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, normal));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, joint));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(3, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, weight));
    glBindVertexArray(0);

    return {vao, vbo, num_verticies};
}

void SkinnedMesh::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, draw_length);
    glBindVertexArray(0);
}

void Shader::use() { glUseProgram(program_id); }

#define FETCH_SHADER_PROP(name)\
    shader.loc_ ##name = glGetUniformLocation(shader.program_id, #name)

MasterShader MasterShader::init() {
    MasterShader shader;
    shader.program_id = Asset::fetch_shader("MASTER_SHADER")->program_id;

    FETCH_SHADER_PROP(t);
    FETCH_SHADER_PROP(proj);
    FETCH_SHADER_PROP(view);
    FETCH_SHADER_PROP(model);
    FETCH_SHADER_PROP(tex);

    return shader;
}

#define F32_SHADER_PROP(classname, name)\
    void classname::upload_ ##name(f32 f) const { glUniform1f(loc_ ##name, f); }

#define U32_SHADER_PROP(classname, name)\
    void classname::upload_ ##name(u32 f) const { glUniform1i(loc_ ##name, f); }

#define MAT_SHADER_PROP(classname, name)\
    void classname::upload_ ##name(Mat &m) const { glUniformMatrix4fv(loc_ ##name, 1, true, m.data()); }

F32_SHADER_PROP(MasterShader, t);
MAT_SHADER_PROP(MasterShader, proj);
MAT_SHADER_PROP(MasterShader, view);
MAT_SHADER_PROP(MasterShader, model);
U32_SHADER_PROP(MasterShader, tex);

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

Camera *main_camera() {
    return &GAMESTATE()->renderer.main_camera;
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

void Shader::destroy() {
    glDeleteProgram(program_id);
}

Shader Shader::compile(const char *source) {
    auto shader_error_check = [](u32 shader) -> bool {
        i32 success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
            ERROR("Shader error: {}", info_log);
        }
        return success;
    };

    auto program_error_check = [](u32 program) -> bool {
        i32 success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
            ERROR("Program error: {}", info_log);
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

    if (!shader_error_check(vertex_shader)) return {-1};

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

    if (!shader_error_check(fragment_shader)) return {-1};

    i32 program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    if (!program_error_check(program)) {
        glDeleteProgram(program);
        return {-1};
    }

    return {program};
}

Texture Texture::upload(u32 width, u32 height, u32 components, u8 *data, Sampling sampling) {
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum format;
    if (components == 1) format = GL_RED;
    else if (components == 2) format = GL_RG;
    else if (components == 3) format = GL_RGB;
    else if (components == 4) format = GL_RGBA;
    else UNREACHABLE("Invalid number of components ({})", components);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    GLenum gl_sampling;
    if (sampling == Sampling::LINEAR) gl_sampling = GL_LINEAR;
    else if (sampling == Sampling::NEAREST) gl_sampling = GL_NEAREST;
    else UNREACHABLE("Unsupported sampling ({})", components);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_sampling);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_sampling);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return { texture };
}

void Texture::bind(u32 texture_slot) {
    ASSERT(texture_slot < 80, "Invalid texture slots. ({})", texture_slot);
    glActiveTexture(GL_TEXTURE0 + texture_slot);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glActiveTexture(GL_TEXTURE0 + 79); // Hardcoded since it's the "minimum maximum".
}

void Texture::destroy() {
    glDeleteTextures(1, &texture_id);
}

bool init(GameState *gs, int width, int height) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        ERROR("Failed to initalize SDL \"{}\"", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    gs->renderer.width = width;
    gs->renderer.height = height;

    gs->window = SDL_CreateWindow("SMEK - The new begining",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

    if (gs->window == NULL) {
        ERROR("Failed to create OpenGL window \"{}\"", SDL_GetError());
        return false;
    }

    gs->gl_context = SDL_GL_CreateContext(gs->window);
    SDL_GL_MakeCurrent(gs->window, gs->gl_context);
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        ERROR("Failed to load OpenGL function.");
        return false;
    }

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

bool reload(GameState *gs) {
    SDL_GL_MakeCurrent(gs->window, gs->gl_context);

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        ERROR("Failed to reload OpenGL function.");
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
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, position));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, color));
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
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, color));

    glPointSize(10.0);
    glDrawArrays(GL_TRIANGLES, 0, size);
    glBindVertexArray(0);
}

void DebugPrimitive::clear() {
    size = 0;
}

void push_debug_triangle(Vec3 p1, Vec4 c1, Vec3 p2, Vec4 c2, Vec3 p3, Vec4 c3) {
    Renderer *renderer = &GAMESTATE()->renderer;
    while (!renderer->primitives[renderer->first_empty].push_triangle({p1, c1}, {p2, c2}, {p3, c3})) {
        if (++renderer->first_empty == renderer->primitives.size())
            renderer->primitives.push_back(DebugPrimitive::init());
    }
}

void draw_primitivs() {
    main_camera()->upload(debug_shader());
    std::vector<DebugPrimitive> *primitives = &GAMESTATE()->renderer.primitives;
    for (DebugPrimitive &p : *primitives) {
        p.draw();
        p.clear();
    }
    GAMESTATE()->renderer.first_empty = 0;
}

} // namespace GFX

