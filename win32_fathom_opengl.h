#ifndef WIN32_FATHOM_OPENGL_H
#define WIN32_FATHOM_OPENGL_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] Win32 OpenGL functions
 * #############################################################################
 */
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_ALPHA_BITS_ARB 0x201B
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002

#define WIN32_OPENGL_API(r) __declspec(dllimport) r __stdcall

typedef void *(*PROC)(void);

/* clang-format off */
WIN32_OPENGL_API(void *) wglCreateContext(void *unnamedParam1);
WIN32_OPENGL_API(void *) wglGetCurrentContext(void);
WIN32_OPENGL_API(void *) wglGetCurrentDC(void);
WIN32_OPENGL_API(i32)    wglDeleteContext(void *unnamedParam1);
WIN32_OPENGL_API(i32)    wglMakeCurrent(void *unnamedParam1, void *unnamedParam2);
WIN32_OPENGL_API(PROC)   wglGetProcAddress(s8 *unnamedParam1);
/* clang-format on */

typedef i32 (*PFNWGLCHOOSEPIXELFORMATARBPROC)(void *hdc, i32 *piAttribIList, f32 *pfAttribFList, u32 nMaxFormats, i32 *piFormats, u32 *nNumFormats);
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

typedef void *(*PFNWGLCREATECONTEXTATTRIBSARBPROC)(void *hDC, void *hShareContext, i32 *attribList);
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

typedef i32 (*PFNWGLSWAPINTERVALEXTPROC)(i32 interval);
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

/* #############################################################################
 * # [SECTION] WIN32 OpenGL Function Loader
 * #############################################################################
 */
typedef PROC (*win32_fathom_opengl_function_loader)(s8 *function_name);

FATHOM_API FATHOM_INLINE u8 win32_fathom_opengl_load_functions(win32_fathom_opengl_function_loader load)
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
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)load("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)load("wglCreateContextAttribsARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)load("wglSwapIntervalEXT");
#pragma GCC diagnostic pop

    return 1;
}

#endif /* WIN32_FATHOM_OPENGL_H */