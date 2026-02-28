#ifndef FATHOM_OPENGL_H
#define FATHOM_OPENGL_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] OpenGL API
 * #############################################################################
 */
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

/* OpenGL 1.1 functions */
typedef void (*PFNGLCLEARCOLORPROC)(f32 red, f32 green, f32 blue, f32 alpha);
static PFNGLCLEARCOLORPROC glClearColor;

typedef void (*PFNGLCLEARPROC)(u32 mask);
static PFNGLCLEARPROC glClear;

typedef void (*PFNGLVIEWPORTPROC)(i32 x, i32 y, i32 width, i32 height);
static PFNGLVIEWPORTPROC glViewport;

typedef void (*PFNGLENABLEPROC)(u32 cap);
static PFNGLENABLEPROC glEnable;

typedef void (*PFNGLDISABLEPROC)(u32 cap);
static PFNGLDISABLEPROC glDisable;

typedef u8 *(*PFNGLGETSTRINGPROC)(u32 name);
static PFNGLGETSTRINGPROC glGetString;

typedef void (*PFNGLGENTEXTURESPROC)(i32 n, u32 *textures);
static PFNGLGENTEXTURESPROC glGenTextures;

typedef void (*PFNGLBINDTEXTUREPROC)(u32 target, u32 texture);
static PFNGLBINDTEXTUREPROC glBindTexture;

typedef void (*PFNGLTEXIMAGE2DPROC)(u32 target, i32 level, i32 internalformat, i32 width, i32 height, i32 border, i32 format, u32 type, const void *pixels);
static PFNGLTEXIMAGE2DPROC glTexImage2D;

typedef void (*PFNGLTEXPARAMETERIPROC)(u32 target, u32 pname, i32 param);
static PFNGLTEXPARAMETERIPROC glTexParameteri;

typedef void (*PFNGLPIXELSTOREIPROC)(u32 pname, i32 param);
static PFNGLPIXELSTOREIPROC glPixelStorei;

typedef void (*PFNGLREADPIXELSPROC)(i32 x, i32 y, i32 width, i32 height, i32 format, i32 type, void *pixels);
static PFNGLREADPIXELSPROC glReadPixels;

typedef void (*PFNGLBLENDFUNCPROC)(u32 sfactor, u32 dfactor);
static PFNGLBLENDFUNCPROC glBlendFunc;

typedef void (*PFNGLGETINTEGERVPROC)(u32 pname, i32 *params);
static PFNGLGETINTEGERVPROC glGetIntegerv;

/* Opengl 1.1+ until current */
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

/* #############################################################################
 * # [SECTION] OpenGL Function Loader
 * #############################################################################
 */
typedef void *(*fathom_opengl_proc)(void);
typedef fathom_opengl_proc (*fathom_opengl_function_loader)(s8 *function_name);

FATHOM_API FATHOM_INLINE u8 fathom_opengl_load_functions(fathom_opengl_function_loader load)
{
    if (!load)
    {
        return 0;
    }

#ifdef _MSC_VER
#pragma warning(disable : 4068)
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

    /* OpenGL 1.1 functions */
    glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
    glClear = (PFNGLCLEARPROC)load("glClear");
    glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
    glEnable = (PFNGLENABLEPROC)load("glEnable");
    glDisable = (PFNGLDISABLEPROC)load("glDisable");
    glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
    glGenTextures = (PFNGLGENTEXTURESPROC)load("glGenTextures");
    glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
    glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
    glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");
    glPixelStorei = (PFNGLPIXELSTOREIPROC)load("glPixelStorei");
    glReadPixels = (PFNGLREADPIXELSPROC)load("glReadPixels");
    glBlendFunc = (PFNGLBLENDFUNCPROC)load("glBlendFunc");
    glGetIntegerv = (PFNGLGETINTEGERVPROC)load("glGetIntegerv");

    /* Opengl 1.1+ until current */
    glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
    glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
    glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
    glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
    glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
    glUniform3f = (PFNGLUNIFORM3FPROC)load("glUniform3f");
    glUniform3i = (PFNGLUNIFORM3IPROC)load("glUniform3i");
    glUniform4f = (PFNGLUNIFORM4FPROC)load("glUniform4f");
    glUniform4fv = (PFNGLUNIFORM4FVPROC)load("glUniform4fv");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)load("glActiveTexture");
    glTexImage3D = (PFNGLTEXIMAGE3DPROC)load("glTexImage3D");
    glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
    glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)load("glVertexAttribIPointer");
    glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)load("glVertexAttribDivisor");
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCED)load("glDrawArraysInstanced");
#pragma GCC diagnostic pop

    return 1;
}

/* #############################################################################
 * # [SECTION] OpenGL Shader Compilation and Creation
 * #############################################################################
 */
typedef void (*fathom_opengl_print)(s8 *string);

static s8 shader_info_log[1024];

FATHOM_API i32 fathom_opengl_shader_compile(s8 *shaderCode, u32 shaderType, fathom_opengl_print print)
{
    u32 shaderId = glCreateShader(shaderType);
    i32 success;

    glShaderSource(shaderId, 1, &shaderCode, 0);
    glCompileShader(shaderId);
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shaderId, 1024, 0, shader_info_log);

        print("[opengl] shader compilation error:\n");
        print(shader_info_log);
        print("\n");

        return -1;
    }

    return (i32)shaderId;
}

FATHOM_API i32 fathom_opengl_shader_create(u32 *shader_program, s8 *shader_vertex_code, s8 *shader_fragment_code, fathom_opengl_print print)
{
    i32 vertex_shader_id;
    i32 fragment_shader_id;
    i32 success;

    vertex_shader_id = fathom_opengl_shader_compile(shader_vertex_code, GL_VERTEX_SHADER, print);

    if (vertex_shader_id == -1)
    {
        return 0;
    }

    fragment_shader_id = fathom_opengl_shader_compile(shader_fragment_code, GL_FRAGMENT_SHADER, print);

    if (fragment_shader_id == -1)
    {
        return 0;
    }

    *shader_program = glCreateProgram();
    glAttachShader(*shader_program, (u32)vertex_shader_id);
    glAttachShader(*shader_program, (u32)fragment_shader_id);
    glLinkProgram(*shader_program);
    glGetProgramiv(*shader_program, GL_LINK_STATUS, &success);
    glDeleteShader((u32)vertex_shader_id);
    glDeleteShader((u32)fragment_shader_id);

    if (!success)
    {
        glGetProgramInfoLog(*shader_program, 1024, 0, shader_info_log);

        print("[opengl] program creation error:\n");
        print(shader_info_log);
        print("\n");

        return 0;
    }

    return 1;
}

#endif /* FATHOM_OPENGL_H */