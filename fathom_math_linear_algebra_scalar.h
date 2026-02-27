#ifndef FATHOM_MATH_LINEAR_ALGEBRA_SCALAR_H
#define FATHOM_MATH_LINEAR_ALGEBRA_SCALAR_H

#include "fathom_math_basic.h"

/* #############################################################################
 * # [SECTION] Linear Algebra Math (Scalar)
 * #############################################################################
 */
typedef struct fathom_vec3
{
    f32 x;
    f32 y;
    f32 z;

} fathom_vec3;

static fathom_vec3 fathom_vec3_zero = {0.0f, 0.0f, 0.0f};

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_init(f32 x, f32 y, f32 z)
{
    fathom_vec3 result;

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_sub(fathom_vec3 a, fathom_vec3 b)
{
    fathom_vec3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_subf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    result.x = a.x - value;
    result.y = a.y - value;
    result.z = a.z - value;

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_addf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    result.x = a.x + value;
    result.y = a.y + value;
    result.z = a.z + value;

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_divf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    result.x = a.x / value;
    result.y = a.y / value;
    result.z = a.z / value;

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_abs(fathom_vec3 a)
{
    fathom_vec3 result;

    result.x = fathom_absf(a.x);
    result.y = fathom_absf(a.y);
    result.z = fathom_absf(a.z);

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_maxf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    result.x = fathom_maxf(a.x, value);
    result.y = fathom_maxf(a.y, value);
    result.z = fathom_maxf(a.z, value);

    return result;
}

FATHOM_API FATHOM_INLINE f32 fathom_vec3_length(fathom_vec3 a)
{
    return fathom_sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

FATHOM_API FATHOM_INLINE f32 fathom_vec3_dot(fathom_vec3 a, fathom_vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_cross(fathom_vec3 a, fathom_vec3 b)
{
    fathom_vec3 result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_normalize(fathom_vec3 a)
{
    f32 length_squared = a.x * a.x + a.y * a.y + a.z * a.z;
    f32 scalar = length_squared > 0.0f ? fathom_invsqrtf(length_squared) : 0.0f;

    fathom_vec3 result;

    result.x = a.x * scalar;
    result.y = a.y * scalar;
    result.z = a.z * scalar;

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

#endif /* FATHOM_MATH_LINEAR_ALGEBRA_SCALAR_H */