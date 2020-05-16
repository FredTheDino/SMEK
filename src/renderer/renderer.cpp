#include "../main.h"
#include "../util/log.h"
#include "opengl.h"
#include "renderer.h"

#include <fstream>

static char *dump_file(const char *file_path) {
    // TODO(ed): Error handling.
    std::ifstream in(file_path, std::ios_base::ate);
    if (in) {
        auto size = in.tellg();
        char *buffer = new char [static_cast<u32>(size) + 1];
        buffer[size] = 0;
        in.seekg(std::ios_base::beg);
        in.read(buffer, size);
        in.close();
        return buffer;
    }
    ERROR("Failed to read \"%s\"", file_path);
    return nullptr;
}

namespace GFX {

Shader shader;

void Shader::use() { GL::UseProgram(program_id); }

Mesh Mesh::init(std::vector<Vec3> points) {
    u32 vao, vbo;

    GL::GenVertexArrays(1, &vao);
    GL::GenBuffers(1, &vbo);

    GL::BindVertexArray(vao);

    GL::BindBuffer(GL::cARRAY_BUFFER, vbo);
    GL::BufferData(GL::cARRAY_BUFFER, sizeof(Vec3) * points.size(), &points[0], GL::cSTATIC_DRAW);

    GL::EnableVertexAttribArray(0);
    GL::VertexAttribPointer(0, 3, GL::cFLOAT, 0, sizeof(Vec3), (void *) 0);

    return {vao, vbo, static_cast<u32>(points.size())};
}

void Mesh::draw() {
    GL::BindVertexArray(vao);
    GL::DrawArrays(GL::cTRIANGLES, 0, draw_length);
    GL::BindVertexArray(0);
}

Shader default_shader() {
    return shader;
}

Shader compile_shader(const char *source) {
    auto shader_error_check = [](u32 shader) -> bool {
        i32 success;
        GL::GetShaderiv(shader, GL::cCOMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            GL::GetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
            ERROR("Shader error: %s", info_log);
        }
        return success;
    };

    auto program_error_check = [](u32 program) -> bool {
        i32 success;
        GL::GetProgramiv(program, GL::cLINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            GL::GetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
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

    // TODO(ed): Defer would be nice here.

    u32 vertex_shader = GL::CreateShader(GL::cVERTEX_SHADER);
    GL::ShaderSource(vertex_shader, sizeof(vertex_source) / sizeof(vertex_source[0]), vertex_source, NULL);
    GL::CompileShader(vertex_shader);

    if (!shader_error_check(vertex_shader)) {
        GL::DeleteShader(vertex_shader);
        return {-1};
    }

    // Fragment
    const char *fragment_source[] = {
        "#version 330\n"
        "#define FRAG\n",
        source
    };

    u32 fragment_shader = GL::CreateShader(GL::cFRAGMENT_SHADER);
    GL::ShaderSource(fragment_shader, sizeof(fragment_source) / sizeof(fragment_source[0]), fragment_source, NULL);
    GL::CompileShader(fragment_shader);

    if (!shader_error_check(fragment_shader)) {
        GL::DeleteShader(vertex_shader);
        GL::DeleteShader(fragment_shader);
        return {-1};
    }

    i32 program = GL::CreateProgram();
    GL::AttachShader(program, vertex_shader);
    GL::AttachShader(program, fragment_shader);
    GL::LinkProgram(program);

    GL::DeleteShader(vertex_shader);
    GL::DeleteShader(fragment_shader);

    if (!program_error_check(program)) {
        GL::DeleteProgram(program);
        return {-1};
    }

    return {program};
}

bool init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        ERROR("Failed to initalize SDL \"%s\"", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    const int width = 640;
    const int height = 480;
    _global_gs.window = SDL_CreateWindow("SMEK - The new begining",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_OPENGL);

    if (_global_gs.window == NULL) {
        ERROR("Failed to create OpenGL window \"%s\"", SDL_GetError());
        return false;
    }

    _global_gs.gl_context = SDL_GL_CreateContext(_global_gs.window);

    if (!GL::load_procs()) {
        ERROR("Failed to load OpenGL function.");
        return false;
    }

    // TODO(ed): Error checking.
    shader = compile_shader(dump_file("master.glsl"));
    return true;
}

void deinit() {
    SDL_Quit();
}

} // namespace GFX

