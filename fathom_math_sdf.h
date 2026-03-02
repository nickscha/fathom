#ifndef FATHOM_MATH_SDF_H
#define FATHOM_MATH_SDF_H

#include "fathom_math_linear_algebra.h"

/* #############################################################################
 * # [SECTION] Signed Distance Functions
 * #############################################################################
 */
FATHOM_API FATHOM_INLINE f32 fathom_sdf_sphere(fathom_vec3 pos, f32 radius)
{
    return fathom_vec3_length(pos) - radius;
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_octahedron(fathom_vec3 pos, f32 scale)
{
    pos = fathom_vec3_abs(pos);
    return (pos.x + pos.y + pos.z - scale) * 0.57735027f;
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_box(fathom_vec3 pos, fathom_vec3 base)
{
    fathom_vec3 q = fathom_vec3_sub(fathom_vec3_abs(pos), base);

    return fathom_vec3_length(fathom_vec3_maxf(q, 0.0f)) + fathom_minf(fathom_maxf(q.x, fathom_maxf(q.y, q.z)), 0.0f);
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_box_rounded(fathom_vec3 pos, fathom_vec3 base, f32 radius)
{
    fathom_vec3 q = fathom_vec3_addf(fathom_vec3_sub(fathom_vec3_abs(pos), base), radius);

    return fathom_vec3_length(fathom_vec3_maxf(q, 0.0f)) + fathom_minf(fathom_maxf(q.x, fathom_maxf(q.y, q.z)), 0.0f) - radius;
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_box_frame(fathom_vec3 pos, fathom_vec3 base, f32 edge_thickness)
{
    fathom_vec3 p = fathom_vec3_sub(fathom_vec3_abs(pos), base);
    fathom_vec3 q = fathom_vec3_subf(fathom_vec3_abs(fathom_vec3_addf(p, edge_thickness)), edge_thickness);

    return fathom_minf(fathom_minf(
                           fathom_vec3_length(fathom_vec3_maxf(fathom_vec3_init(p.x, q.y, q.z), 0.0f)) + fathom_minf(fathom_maxf(p.x, fathom_maxf(q.y, q.z)), 0.0f),
                           fathom_vec3_length(fathom_vec3_maxf(fathom_vec3_init(q.x, p.y, q.z), 0.0f)) + fathom_minf(fathom_maxf(q.x, fathom_maxf(p.y, q.z)), 0.0f)),
                       fathom_vec3_length(fathom_vec3_maxf(fathom_vec3_init(q.x, q.y, p.z), 0.0f)) + fathom_minf(fathom_maxf(q.x, fathom_maxf(q.y, p.z)), 0.0f));
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_ellipsoid(fathom_vec3 pos, fathom_vec3 radius)
{
    f32 k0 = fathom_vec3_length(fathom_vec3_div(pos, radius));
    f32 k1 = fathom_vec3_length(fathom_vec3_div(pos, fathom_vec3_mul(radius, radius)));
    return k0 * (k0 - 1.0f) / k1;
}

/* #############################################################################
 * # [SECTION] Signed Distance Operations Exact
 * #############################################################################
 */
FATHOM_API FATHOM_INLINE f32 fathom_sdf_op_union(f32 a, f32 b)
{
    return fathom_minf(a, b);
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_op_subtract(f32 a, f32 b)
{
    return fathom_maxf(-a, b);
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_op_intersect(f32 a, f32 b)
{
    return fathom_maxf(a, b);
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_op_xor(f32 a, f32 b)
{
    return fathom_maxf(fathom_minf(a, b), -fathom_maxf(a, b));
}

#endif /* FATHOM_MATH_SDF_H */