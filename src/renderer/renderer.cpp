#include "../game.h"
#include "../util/util.h"
#include "opengl.h"
#include "renderer.h"

namespace GFX {

Camera Camera::init(f32 fov) {
    Camera cam = {};
    cam.set_fov(fov);
    cam.up = Vec3(0, 1, 0);
    cam.forward = Vec3(0, 0, -1);
    cam.position = Vec3(0, 0, 0);
    return cam;
}

void Camera::set_fov(f32 fov) {
    perspective = Mat::perspective(fov, 0.01, 10.0);
}

void Camera::look_at_from(Vec3 from, Vec3 target) {
    position = from;
    look_at(target);
}

void Camera::look_at(Vec3 target) {
    forward = target - position;
}

void Camera::turn(f32 jaw, f32 pitch) {
    forward = Mat::rotate_x(-jaw) * Mat::rotate_y(-pitch) * forward;
}

void Camera::move(Vec3 movement) {
    position = position - movement;
}

void Camera::move_relative(Vec3 movement) {
    Mat view = Mat::look_at(position, position + forward, up);
    Vec4 relative_move = view * Vec4(movement.x, movement.y, movement.z, 0.0);
    position = position - Vec3(relative_move.x, relative_move.y, relative_move.z);
}

template<>
void Camera::upload(const MasterShader &shader) {
    shader.upload_proj(perspective);
    Mat view = Mat::look_at(position, position + forward, up);
    shader.upload_view(view);
}

Mesh Mesh::init(Asset::Model *model) {
    using Asset::Vertex;
    u32 vao, vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * model->num_faces * 3, model->data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, texture));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, normal));

    return {vao, vbo, model->num_faces * 3};;
}

void Mesh::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, draw_length);
    glBindVertexArray(0);
}


void Shader::use() { glUseProgram(program_id); }

#define FETCH_SHADER_PROP(name)\
    shader.loc_ ##name = glGetUniformLocation(shader.program_id, #name)

MasterShader MasterShader::init() {
    MasterShader shader;
    shader.program_id = Shader::compile("MASTER_SHADER").program_id;

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
    return GAMESTATE()->renderer.master_shader;
}

Shader Shader::compile(AssetID source_id) {
    const char *source = Asset::fetch_shader("MASTER_SHADER")->data;
    auto shader_error_check = [](u32 shader) -> bool {
        i32 success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
            ERROR("Shader error: %s", info_log);
        }
        return success;
    };

    auto program_error_check = [](u32 program) -> bool {
        i32 success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
            ERROR("Program error: %s", info_log);
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
    else UNREACHABLE("Invalid number of components (%d)", components);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    GLenum gl_sampling;
    if (sampling == Sampling::LINEAR) gl_sampling = GL_LINEAR;
    else if (sampling == Sampling::NEAREST) gl_sampling = GL_NEAREST;
    else UNREACHABLE("Unsupported sampling (%d)", components);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_sampling);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_sampling);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return { texture };
}

Texture Texture::upload(Asset::Image *image, Sampling sampling) {
    return upload(image->width, image->height, image->components, image->data, sampling);
}

void Texture::bind(u32 texture_slot) {
    ASSERT(texture_slot < 80, "Invalid texture slots. (%d)", texture_slot);
    glActiveTexture(GL_TEXTURE0 + texture_slot);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glActiveTexture(GL_TEXTURE0 + 79); // Hardcoded since it's the "minimum maximum".
}

bool init(GameState *gs, int width, int height) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        ERROR("Failed to initalize SDL \"%s\"", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    gs->window = SDL_CreateWindow("SMEK - The new begining",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

    if (gs->window == NULL) {
        ERROR("Failed to create OpenGL window \"%s\"", SDL_GetError());
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

} // namespace GFX

