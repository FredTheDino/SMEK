#pragma once

namespace GL {

constexpr u32 ARRAY_BUFFER = 0x8892;
constexpr u32 BLEND = 0x0BE2;
constexpr u32 COMPILE_STATUS = 0x8B81;
constexpr u32 DEBUG_OUTPUT = 0x92E0;
constexpr u32 DEBUG_TYPE_ERROR = 0x824C;
constexpr u32 DEPTH_BUFFER_BIT = 0x00000100;
constexpr u32 COLOR_BUFFER_BIT = 0x00004000;
constexpr u32 FLOAT = 0x1406;
constexpr u32 FRAGMENT_SHADER = 0x8B30;
constexpr u32 FRAMEBUFFER = 0x8D40;
constexpr u32 FRAMEBUFFER_COMPLETE = 0x8CD5;
constexpr u32 INT = 0x1404;
constexpr u32 LINK_STATUS = 0x8B82;
// constexpr u32 RED = 0x1903; // If we want this, we'll have to rename a macro.
constexpr u32 RENDERBUFFER = 0x8D41;
constexpr u32 RG = 0x8227;
constexpr u32 RGB = 0x1907;
constexpr u32 RGBA = 0x1908;
constexpr u32 SRC_ALPHA = 0x0302;
constexpr u32 STATIC_DRAW = 0x88E4;
constexpr u32 STREAM_DRAW = 0x88E0;
constexpr u32 TEXTURE0 = 0x84C0;
constexpr u32 TEXTURE1 = 0x84C1;
constexpr u32 TEXTURE_2D = 0x0DE1;
constexpr u32 TEXTURE_2D_ARRAY = 0x8C1A;
constexpr u32 TRIANGLES = 0x0004;
constexpr u32 UNIFORM_BUFFER = 0x8A11;
constexpr u32 UNSIGNED_BYTE = 0x1401;
constexpr u32 VERTEX_SHADER = 0x8B31;

#define GL_FUNC(ret, name, ...)\
    typedef ret (* _ ## name ## _func)(__VA_ARGS__);\
    _ ## name ## _func name = nullptr;

using GLenum = u32;

GL_FUNC(void, GenTextures, u32, u32 *);
GL_FUNC(void, BindTexture, GLenum, u32);
GL_FUNC(void, TexImage2D, GLenum, i32, i32, u32, u32, i32, GLenum, GLenum, const void *);
GL_FUNC(void, GenerateMipmap, GLenum);
GL_FUNC(void, TexParameteri, GLenum, GLenum, i32);
GL_FUNC(void, ActiveTexture, GLenum);

GL_FUNC(void, Enable, GLenum);
GL_FUNC(void, Disable, GLenum);
GL_FUNC(void, BlendFunc, GLenum, GLenum);

GL_FUNC(void, ClearColor, f32, f32, f32, f32)
GL_FUNC(void, Clear, u32)

GL_FUNC(u32, CreateShader, GLenum);
GL_FUNC(void, CompileShader, u32);
GL_FUNC(void, GetShaderiv, u32, GLenum, i32 *);
GL_FUNC(void, GetShaderInfoLog, u32, u32, u32 *, char *);

GL_FUNC(void, UseProgram, u32);
GL_FUNC(u32, CreateProgram, void);
GL_FUNC(void, AttachShader, u32, u32);
GL_FUNC(void, LinkProgram, u32);
GL_FUNC(void, GetProgramiv, u32, GLenum, i32 *);
GL_FUNC(void, GetProgramInfoLog, u32, u32, u32 *, char *);
GL_FUNC(void, DeleteProgram, u32);
GL_FUNC(void, DeleteShader, u32);

GL_FUNC(i32, GetUniformLocation, u32, const char *);
GL_FUNC(void, Uniform1i, i32, i32);
GL_FUNC(void, Uniform1f, i32, f32);
GL_FUNC(void, Uniform2f, i32, f32, f32);
GL_FUNC(void, Uniform3f, i32, f32, f32, f32);
GL_FUNC(void, Uniform4f, i32, f32, f32, f32, f32);
GL_FUNC(void, UniformMatrix4fv, i32, u32, b8, const f32 *);

GL_FUNC(void, GenVertexArrays, u32, u32 *);
GL_FUNC(void, BindVertexArray, u32);
GL_FUNC(void, EnableVertexAttribArray, u32);
GL_FUNC(void, VertexAttribPointer, u32, i32, GLenum, b8, u32, const void *);
GL_FUNC(void, DrawElements, GLenum, u32, GLenum, const void *);

GL_FUNC(void, GenBuffers, u32, u32 *);
GL_FUNC(void, BindBuffer, GLenum, u32);
GL_FUNC(void, BufferData, GLenum, u64, const void *, GLenum);

#define GL_LOAD(name)\
    name = (_ ## name ## _func) SDL_GL_GetProcAddress("gl" #name);\
    if (!name) { ERROR("Failed to load OpenGL func: \"" #name "\""); success = false; }
// The return address of the function isn't allways garanteed to be correct, or even
// be NULL if it doesn't exist, so this if statement doesn't really work... But it's
// something.

bool load_procs() {
    bool success = true;

    GL_LOAD(GenTextures);
    GL_LOAD(BindTexture);
    GL_LOAD(TexImage2D);
    GL_LOAD(GenerateMipmap);
    GL_LOAD(TexParameteri);
    GL_LOAD(ActiveTexture);

    GL_LOAD(Enable);
    GL_LOAD(Disable);
    GL_LOAD(BlendFunc);

    GL_LOAD(ClearColor);
    GL_LOAD(Clear);

    GL_LOAD(CreateShader);
    GL_LOAD(CompileShader);
    GL_LOAD(GetShaderiv);
    GL_LOAD(GetShaderInfoLog);

    GL_LOAD(UseProgram);
    GL_LOAD(CreateProgram);
    GL_LOAD(AttachShader);
    GL_LOAD(LinkProgram);
    GL_LOAD(GetProgramiv);
    GL_LOAD(GetProgramInfoLog);
    GL_LOAD(DeleteProgram);
    GL_LOAD(DeleteShader);

    GL_LOAD(GetUniformLocation);
    GL_LOAD(Uniform1i);
    GL_LOAD(Uniform1f);
    GL_LOAD(Uniform2f);
    GL_LOAD(Uniform3f);
    GL_LOAD(Uniform4f);
    GL_LOAD(UniformMatrix4fv);

    GL_LOAD(GenVertexArrays);
    GL_LOAD(BindVertexArray);
    GL_LOAD(EnableVertexAttribArray);
    GL_LOAD(VertexAttribPointer);
    GL_LOAD(DrawElements);

    GL_LOAD(GenBuffers);
    GL_LOAD(BindBuffer);
    GL_LOAD(BufferData);

    return success;
}

}
