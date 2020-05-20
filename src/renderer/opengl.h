#pragma once
#include "../math/types.h"

namespace GL {

constexpr u32 cARRAY_BUFFER = 0x8892;
constexpr u32 cBLEND = 0x0BE2;
constexpr u32 cCOMPILE_STATUS = 0x8B81;
constexpr u32 cDEBUG_OUTPUT = 0x92E0;
constexpr u32 cDEBUG_TYPE_ERROR = 0x824C;
constexpr u32 cDEPTH_BUFFER_BIT = 0x00000100;
constexpr u32 cDEPTH_TEST = 0x0B71;
constexpr u32 cCOLOR_BUFFER_BIT = 0x00004000;
constexpr u32 cFLOAT = 0x1406;
constexpr u32 cFRAGMENT_SHADER = 0x8B30;
constexpr u32 cFRAMEBUFFER = 0x8D40;
constexpr u32 cFRAMEBUFFER_COMPLETE = 0x8CD5;
constexpr u32 cINT = 0x1404;
constexpr u32 cLINK_STATUS = 0x8B82;
constexpr u32 cRED = 0x1903;
constexpr u32 cRENDERBUFFER = 0x8D41;
constexpr u32 cRG = 0x8227;
constexpr u32 cRGB = 0x1907;
constexpr u32 cRGBA = 0x1908;
constexpr u32 cSRC_ALPHA = 0x0302;
constexpr u32 cSTATIC_DRAW = 0x88E4;
constexpr u32 cSTREAM_DRAW = 0x88E0;
constexpr u32 cTEXTURE0 = 0x84C0;
constexpr u32 cTEXTURE1 = 0x84C1;
constexpr u32 cTEXTURE_2D = 0x0DE1;
constexpr u32 cTEXTURE_2D_ARRAY = 0x8C1A;
constexpr u32 cTRIANGLES = 0x0004;
constexpr u32 cUNIFORM_BUFFER = 0x8A11;
constexpr u32 cUNSIGNED_BYTE = 0x1401;
constexpr u32 cVERTEX_SHADER = 0x8B31;

constexpr u32 cNEAREST = 0x2600;
constexpr u32 cLINEAR = 0x2601;
constexpr u32 cTEXTURE_MIN_FILTER = 0x2800;
constexpr u32 cTEXTURE_MAG_FILTER = 0x2801;
constexpr u32 cTEXTURE_WRAP_S = 0x2802;
constexpr u32 cTEXTURE_WRAP_T = 0x2803;
constexpr u32 cCLAMP_TO_EDGE = 0x812F;

#ifdef _SMEK_GL_IMPLEMENT
#define GL_FUNC(ret, name, ...)\
    typedef ret (* _ ## name ## _func)(__VA_ARGS__);\
    _ ## name ## _func name = nullptr;
#else
#define GL_FUNC(ret, name, ...)\
    typedef ret (* _ ## name ## _func)(__VA_ARGS__);\
    extern _ ## name ## _func name;
#endif

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
GL_FUNC(void, ShaderSource, u32, u32, const char **, const i32 *);
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
GL_FUNC(void, DrawArrays, GLenum, u32, u32);

GL_FUNC(void, GenBuffers, u32, u32 *);
GL_FUNC(void, BindBuffer, GLenum, u32);
GL_FUNC(void, BufferData, GLenum, u64, const void *, GLenum);

GL_FUNC(void, ReadPixels, i32, i32, u32, u32, GLenum, GLenum, void *);
GL_FUNC(void, Finish, void);

// Loads the actual OpenGL function pointers.
bool load_procs();

}
