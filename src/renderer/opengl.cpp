#include "SDL2/SDL.h"
#include "../util/log.h"

#define _SMEK_GL_IMPLEMENT
#include "opengl.h"

namespace GL {

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
    GL_LOAD(ShaderSource);
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
    GL_LOAD(DrawArrays);

    GL_LOAD(GenBuffers);
    GL_LOAD(BindBuffer);
    GL_LOAD(BufferData);

    GL_LOAD(ReadPixels);

    return success;
}

} // namespace GL
