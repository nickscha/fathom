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

/* #############################################################################
 * # [SECTION] Linear Algebra Math (General, No faster SIMD substitution)
 * #############################################################################
 */
FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_divf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    result.x = a.x / value;
    result.y = a.y / value;
    result.z = a.z / value;

    return result;
}


#define FATHOM_MAT2X2_ELEMENT_COUNT 4

#ifdef FATHOM_MAT_ROW_MAJOR_ORDER
#define FATHOM_MAT2X2_AT(row, col) ((row) * 2 + (col)) /* Row-major order */
#else
#define FATHOM_MAT2X2_AT(row, col) ((col) * 2 + (row)) /* Column-major order */
#endif

typedef struct fathom_mat2x2
{
    f32 e[FATHOM_MAT2X2_ELEMENT_COUNT];

} fathom_mat2x2;

FATHOM_API FATHOM_INLINE fathom_mat2x2 fathom_mat2x2_init(f32 m00, f32 m01, f32 m10, f32 m11)
{
    fathom_mat2x2 result;

    result.e[FATHOM_MAT2X2_AT(0, 0)] = m00;
    result.e[FATHOM_MAT2X2_AT(0, 1)] = m01;
    result.e[FATHOM_MAT2X2_AT(1, 0)] = m10;
    result.e[FATHOM_MAT2X2_AT(1, 1)] = m11;

    return result;
}

FATHOM_API FATHOM_INLINE fathom_mat2x2 fathom_mat2x2_rot2d(f32 angle)
{
    f32 s = fathom_sinf(angle);
    f32 c = fathom_cosf(angle);

    return fathom_mat2x2_init(c, -s, s, c);
}

FATHOM_API FATHOM_INLINE void fathom_vec2_mul_mat2x2(f32 *a, f32 *b, fathom_mat2x2 m)
{
    f32 x = *a;
    f32 y = *b;

    *a = x * m.e[FATHOM_MAT2X2_AT(0, 0)] + y * m.e[FATHOM_MAT2X2_AT(1, 0)];
    *b = x * m.e[FATHOM_MAT2X2_AT(0, 1)] + y * m.e[FATHOM_MAT2X2_AT(1, 1)];
}

#endif /* FATHOM_MATH_LINEAR_ALGEBRA_H */