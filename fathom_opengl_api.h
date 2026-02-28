#ifndef FATHOM_OPENGL_API_H
#define FATHOM_OPENGL_API_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] OpenGL API
 * #############################################################################
 */
/* OpenGL functions directly part of opengl32 lib */
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_UNSIGNED_SHORT 0x1403
#define GL_FLOAT 0x1406
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_FAN 0x0006
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#define GL_MULTISAMPLE 0x809D
#define GL_COMPILE_STATUS 0x8B81
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_LINK_STATUS 0x8B82
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_STREAM_DRAW 0x88E0
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_ARRAY_BUFFER 0x8892
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_3D 0x806F
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_RGB 0x1907
#define GL_RED 0x1903
#define GL_RED_INTEGER 0x8D94
#define GL_R8 0x8229
#define GL_R8UI 0x8232
#define GL_R8_SNORM 0x8F94
#define GL_R16UI 0x8234
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303

typedef u32 (*PFNGLCREATESHADERPROC)(u32 shaderType);
static PFNGLCREATESHADERPROC glCreateShader;

typedef u32 (*PFNGLCREATEPROGRAMPROC)(void);
static PFNGLCREATEPROGRAMPROC glCreateProgram;

typedef void (*PFNGLDELETEPROGRAMPROC)(u32 program);
static PFNGLDELETEPROGRAMPROC glDeleteProgram;

typedef void (*PFNGLATTACHSHADERPROC)(u32 program, u32 shader);
static PFNGLATTACHSHADERPROC glAttachShader;

typedef void (*PFNGLSHADERSOURCEPROC)(u32 shader, i32 count, s8 **string, i32 *length);
static PFNGLSHADERSOURCEPROC glShaderSource;

typedef void (*PFNGLCOMPILESHADERPROC)(u32 shader);
static PFNGLCOMPILESHADERPROC glCompileShader;

typedef void (*PFNGLGETSHADERIVPROC)(u32 shader, u32 pname, i32 *params);
static PFNGLGETSHADERIVPROC glGetShaderiv;

typedef void (*PFNGLGETSHADERINFOLOGPROC)(u32 shader, i32 maxLength, i32 *length, s8 *infoLog);
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

typedef void (*PFNGLLINKPROGRAMPROC)(u32 program);
static PFNGLLINKPROGRAMPROC glLinkProgram;

typedef void (*PFNGLGETPROGRAMIVPROC)(u32 program, u32 pname, i32 *params);
static PFNGLGETPROGRAMIVPROC glGetProgramiv;

typedef void (*PFNGLGETPROGRAMINFOLOGPROC)(u32 program, i32 maxLength, i32 *length, s8 *infoLog);
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

typedef void (*PFNGLDELETESHADERPROC)(u32 shader);
static PFNGLDELETESHADERPROC glDeleteShader;

typedef void (*PFNGLDRAWARRAYSPROC)(u32 mode, i32 first, i32 count);
static PFNGLDRAWARRAYSPROC glDrawArrays;

typedef void (*PFNGLUSEPROGRAMPROC)(u32 program);
static PFNGLUSEPROGRAMPROC glUseProgram;

typedef void (*PFNGLGENVERTEXARRAYSPROC)(i32 n, u32 *arrays);
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;

typedef void (*PFNGLBINDVERTEXARRAYPROC)(u32 array);
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

typedef i32 (*PFNGLGETUNIFORMLOCATIONPROC)(u32 program, s8 *name);
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

typedef void (*PFNGLUNIFORM1FPROC)(i32 location, f32 v0);
static PFNGLUNIFORM1FPROC glUniform1f;

typedef void (*PFNGLUNIFORM1IPROC)(i32 location, i32 v0);
static PFNGLUNIFORM1IPROC glUniform1i;

typedef void (*PFNGLUNIFORM3FPROC)(i32 location, f32 v0, f32 v1, f32 v2);
static PFNGLUNIFORM3FPROC glUniform3f;

typedef void (*PFNGLUNIFORM3IPROC)(i32 location, i32 v0, i32 v1, i32 v2);
static PFNGLUNIFORM3IPROC glUniform3i;

typedef void (*PFNGLUNIFORM4FPROC)(i32 location, f32 v0, f32 v1, f32 v2, f32 v3);
static PFNGLUNIFORM4FPROC glUniform4f;

typedef void (*PFNGLUNIFORM4FVPROC)(i32 location, i32 count, f32 *value);
static PFNGLUNIFORM4FVPROC glUniform4fv;

typedef void (*PFNGLACTIVETEXTUREPROC)(u32 texture);
static PFNGLACTIVETEXTUREPROC glActiveTexture;

typedef void (*PFNGLTEXIMAGE3DPROC)(u32 target, i32 level, i32 internalFormat, i32 width, i32 height, i32 depth, i32 border, u32 format, u32 type, void *data);
static PFNGLTEXIMAGE3DPROC glTexImage3D;

typedef void (*PFNGLGENBUFFERSPROC)(i32 n, u32 *buffers);
static PFNGLGENBUFFERSPROC glGenBuffers;

typedef void (*PFNGLBINDBUFFERPROC)(u32 target, u32 buffer);
static PFNGLBINDBUFFERPROC glBindBuffer;

typedef void (*PFNGLBUFFERDATAPROC)(u32 target, i32 size, void *data, u32 usage);
static PFNGLBUFFERDATAPROC glBufferData;

typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC)(u32 index);
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

typedef void (*PFNGLVERTEXATTRIBPOINTERPROC)(u32 index, i32 size, u32 type, u8 normalized, i32 stride, void *pointer);
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

typedef void (*PFNGLVERTEXATTRIBIPOINTERPROC)(u32 index, i32 size, u32 type, i32 stride, void *pointer);
static PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;

typedef void (*PFNGLVERTEXATTRIBDIVISORPROC)(u32 index, u32 divisor);
static PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;

typedef void (*PFNGLDRAWARRAYSINSTANCED)(i32 mode, i32 first, i32 count, u32 primcount);
static PFNGLDRAWARRAYSINSTANCED glDrawArraysInstanced;

#endif /* FATHOM_OPENGL_API_H */