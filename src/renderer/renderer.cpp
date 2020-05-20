#include "../main.h"
#include "../util/util.h"
#include "opengl.h"
#include "renderer.h"

namespace GFX {

Shader shader;

void Shader::use() { GL::UseProgram(program_id); }

Mesh Mesh::init(Asset::Model *model) {
    using Asset::Vertex;
    u32 vao, vbo;

    GL::GenVertexArrays(1, &vao);
    GL::GenBuffers(1, &vbo);

    GL::BindVertexArray(vao);

    GL::BindBuffer(GL::cARRAY_BUFFER, vbo);
    GL::BufferData(GL::cARRAY_BUFFER, sizeof(Vertex) * model->num_faces * 3, model->data, GL::cSTATIC_DRAW);

    GL::EnableVertexAttribArray(0);
    GL::VertexAttribPointer(0, 3, GL::cFLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, position));

    GL::EnableVertexAttribArray(1);
    GL::VertexAttribPointer(1, 2, GL::cFLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, texture));

    GL::EnableVertexAttribArray(2);
    GL::VertexAttribPointer(2, 3, GL::cFLOAT, 0, sizeof(Vertex), (void *) offsetof(Vertex, normal));

    return {vao, vbo, model->num_faces * 3};;
}

void Mesh::draw() {
    GL::BindVertexArray(vao);
    GL::DrawArrays(GL::cTRIANGLES, 0, draw_length);
    GL::BindVertexArray(0);
}

Shader default_shader() {
    return shader;
}

Shader Shader::compile(const char *source) {
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

    u32 vertex_shader = GL::CreateShader(GL::cVERTEX_SHADER);
    defer { GL::DeleteShader(vertex_shader); };

    GL::ShaderSource(vertex_shader, sizeof(vertex_source) / sizeof(vertex_source[0]), vertex_source, NULL);
    GL::CompileShader(vertex_shader);

    if (!shader_error_check(vertex_shader)) return {-1};

    // Fragment
    const char *fragment_source[] = {
        "#version 330\n"
        "#define FRAG\n",
        source
    };

    u32 fragment_shader = GL::CreateShader(GL::cFRAGMENT_SHADER);
    defer { GL::DeleteShader(fragment_shader); };
    GL::ShaderSource(fragment_shader, sizeof(fragment_source) / sizeof(fragment_source[0]), fragment_source, NULL);
    GL::CompileShader(fragment_shader);

    if (!shader_error_check(fragment_shader)) return {-1};

    i32 program = GL::CreateProgram();
    GL::AttachShader(program, vertex_shader);
    GL::AttachShader(program, fragment_shader);
    GL::LinkProgram(program);

    if (!program_error_check(program)) {
        GL::DeleteProgram(program);
        return {-1};
    }

    return {program};
}

Texture Texture::upload(u32 width, u32 height, u32 components, u8 *data, Sampling sampling) {
    u32 texture;
    GL::GenTextures(1, &texture);
    GL::BindTexture(GL::cTEXTURE_2D, texture);

    GL::GLenum format;
    if (components == 1) format = GL::cRED;
    else if (components == 2) format = GL::cRG;
    else if (components == 3) format = GL::cRGB;
    else if (components == 4) format = GL::cRGBA;
    else UNREACHABLE("Invalid number of components (%d)", components);
    GL::TexImage2D(GL::cTEXTURE_2D, 0, GL::cRGBA, width, height, 0, format, GL::cUNSIGNED_BYTE, data);
    GL::GLenum gl_sampling;
    if (sampling == Sampling::LINEAR) gl_sampling = GL::cLINEAR;
    else if (sampling == Sampling::NEAREST) gl_sampling = GL::cNEAREST;
    else UNREACHABLE("Unsupported sampling (%d)", components);

    GL::TexParameteri(GL::cTEXTURE_2D, GL::cTEXTURE_MIN_FILTER, gl_sampling);
    GL::TexParameteri(GL::cTEXTURE_2D, GL::cTEXTURE_MAG_FILTER, gl_sampling);

    GL::TexParameteri(GL::cTEXTURE_2D, GL::cTEXTURE_WRAP_S, GL::cCLAMP_TO_EDGE);
    GL::TexParameteri(GL::cTEXTURE_2D, GL::cTEXTURE_WRAP_T, GL::cCLAMP_TO_EDGE);
    return { texture };
}

Texture Texture::upload(Asset::Image *image, Sampling sampling) {
    return upload(image->width, image->height, image->components, image->data, sampling);
}

void Texture::bind(u32 texture_slot) {
    ASSERT(texture_slot < 80, "Invalid texture slots. (%d)", texture_slot);
    GL::ActiveTexture(GL::cTEXTURE0 + texture_slot);
    GL::BindTexture(GL::cTEXTURE_2D, texture_id);
    GL::ActiveTexture(GL::cTEXTURE0 + 79); // Hardcoded since it's the "minimum maximum".
}

bool init(GameState *gs, const char *shader_source, int width, int height) {
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

    if (!GL::load_procs()) {
        ERROR("Failed to load OpenGL function.");
        return false;
    }

    if (gs == global_gamestate()) {
        GL::ClearColor(0, 0, 0, 0);
        GL::Clear(GL::cCOLOR_BUFFER_BIT);
        SDL_ShowWindow(gs->window);
        SDL_GL_SwapWindow(gs->window);
    }

    shader = Shader::compile(shader_source);

    GL::Enable(GL::cDEPTH_TEST);
    return true;
}

void deinit(GameState *gs) {
    SDL_DestroyWindow(gs->window);
    SDL_GL_DeleteContext(gs->gl_context);

    SDL_Quit();
}

} // namespace GFX

