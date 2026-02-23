#ifndef FATHOM_MATH_LINEAR_ALGEBRA_H
#define FATHOM_MATH_LINEAR_ALGEBRA_H

#include "fathom_math_basic.h"

/* #############################################################################
 * # [SECTION] Linear Algebra Math
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

#endif /* FATHOM_MATH_LINEAR_ALGEBRA_H */