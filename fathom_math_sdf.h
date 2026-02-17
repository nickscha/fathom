#ifndef FATHOM_MATH_SDF_H
#define FATHOM_MATH_SDF_H

#include "fathom_math_linear_algebra.h"

FATHOM_API f32 fathom_sdf_sphere(fathom_vec3 pos, f32 radius)
{
    return fathom_vec3_length(pos) - radius;
}

FATHOM_API f32 fathom_sdf_box(fathom_vec3 pos, fathom_vec3 base)
{
    fathom_vec3 q = fathom_vec3_sub(fathom_vec3_abs(pos), base);

    return fathom_vec3_length(fathom_vec3_maxf(q, 0.0f)) + fathom_minf(fathom_maxf(q.x, fathom_maxf(q.y, q.z)), 0.0f);
}

FATHOM_API f32 fathom_sdf_box_rounded(fathom_vec3 pos, fathom_vec3 base, f32 radius)
{
    fathom_vec3 q = fathom_vec3_addf(fathom_vec3_sub(fathom_vec3_abs(pos), base), radius);

    return fathom_vec3_length(fathom_vec3_maxf(q, 0.0f)) + fathom_minf(fathom_maxf(q.x, fathom_maxf(q.y, q.z)), 0.0f) - radius;
}

#endif /* FATHOM_MATH_SDF_H */