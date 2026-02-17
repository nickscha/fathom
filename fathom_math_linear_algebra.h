#ifndef FATHOM_MATH_LINEAR_ALGEBRA_H
#define FATHOM_MATH_LINEAR_ALGEBRA_H

#include "fathom_types.h"
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
    fathom_vec3 result = a;

    result.x -= b.x;
    result.y -= b.y;
    result.z -= b.z;

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_divf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result = a;

    result.x /= value;
    result.y /= value;
    result.z /= value;

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

#endif /* FATHOM_MATH_LINEAR_ALGEBRA_H */