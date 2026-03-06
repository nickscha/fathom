#ifndef FATHOM_MATH_LINEAR_ALGEBRA_H
#define FATHOM_MATH_LINEAR_ALGEBRA_H

#include "fathom_math_basic.h"

/* #############################################################################
 * # [SECTION] Linear Algebra Math (SIMD Detection)
 * #############################################################################
 */
#ifdef FATHOM_DISABLE_SIMD
#include "fathom_math_linear_algebra_scalar.h"
#elif defined(FATHOM_ARCH_X64)
#include "fathom_math_linear_algebra_sse2.h"
#endif

#define FATHOM_MAT4X4_ELEMENT_COUNT 16

#ifdef FATHOM_MAT_ROW_MAJOR_ORDER
#define FATHOM_MAT4X4_AT(row, col) ((row) * 4 + (col)) /* Row-major order */
#else
#define FATHOM_MAT4X4_AT(row, col) ((col) * 4 + (row)) /* Column-major order */
#endif

typedef struct fathom_mat4x4
{
    f32 e[FATHOM_MAT4X4_ELEMENT_COUNT];
} fathom_mat4x4;

static fathom_mat4x4 fathom_mat4x4_zero =
    {{0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f}};

FATHOM_API FATHOM_INLINE fathom_mat4x4 fathom_mat4x4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    f32 width = right - left;
    f32 height = top - bottom;
    f32 depth = far - near;

    fathom_mat4x4 result = fathom_mat4x4_zero;

    result.e[FATHOM_MAT4X4_AT(0, 0)] = 2.0f / width;
    result.e[FATHOM_MAT4X4_AT(0, 3)] = -(right + left) / width;
    result.e[FATHOM_MAT4X4_AT(1, 1)] = 2.0f / height;
    result.e[FATHOM_MAT4X4_AT(1, 3)] = -(top + bottom) / height;
    result.e[FATHOM_MAT4X4_AT(2, 2)] = -2.0f / depth;
    result.e[FATHOM_MAT4X4_AT(2, 3)] = -(far + near) / depth;
    result.e[FATHOM_MAT4X4_AT(3, 3)] = 1.0f;

    return result;
}

#endif /* FATHOM_MATH_LINEAR_ALGEBRA_H */