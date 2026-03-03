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

    f32 l1 = fathom_vec3_length(fathom_vec3_maxf(fathom_vec3_init(p.x, q.y, q.z), 0.0f));
    f32 l2 = fathom_vec3_length(fathom_vec3_maxf(fathom_vec3_init(q.x, p.y, q.z), 0.0f));
    f32 l3 = fathom_vec3_length(fathom_vec3_maxf(fathom_vec3_init(q.x, q.y, p.z), 0.0f));

    f32 m1 = fathom_minf(fathom_maxf(p.x, fathom_maxf(q.y, q.z)), 0.0f);
    f32 m2 = fathom_minf(fathom_maxf(q.x, fathom_maxf(p.y, q.z)), 0.0f);
    f32 m3 = fathom_minf(fathom_maxf(q.x, fathom_maxf(q.y, p.z)), 0.0f);

    return fathom_minf(fathom_minf(l1 + m1, l2 + m2), l3 + m3);
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

/* #############################################################################
 * # [SECTION] Signed Distance Operations Smooth
 * #############################################################################
 */
FATHOM_API FATHOM_INLINE f32 fathom_sdf_op_union_smooth(f32 a, f32 b, f32 k)
{
    f32 h = fathom_maxf(k - fathom_absf(a - b), 0.0f) / k;
    return fathom_minf(a, b) - h * h * h * k * (1.0f / 6.0f);
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_op_subtract_smooth(f32 a, f32 b, f32 k)
{
    return -fathom_sdf_op_union_smooth(a, -b, k);
}

FATHOM_API FATHOM_INLINE f32 fathom_sdf_op_intersect_smooth(f32 a, f32 b, f32 k)
{
    return -fathom_sdf_op_union_smooth(-a, -b, k);
}

/* #############################################################################
 * # [SECTION] Signed Distance Operations Symmetry
 * #############################################################################
 */
FATHOM_API FATHOM_INLINE fathom_vec3 fathom_sdf_op_symmetric_x(fathom_vec3 p)
{
    p.x = fathom_absf(p.x);
    return p;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_sdf_op_symmetric_xz(fathom_vec3 p)
{
    p.x = fathom_absf(p.x);
    p.z = fathom_absf(p.z);
    return p;
}

/* #############################################################################
 * # [SECTION] Signed Distance Operations Repetition
 * #############################################################################
 */
FATHOM_API FATHOM_INLINE fathom_vec3 fathom_sdf_op_repeat(fathom_vec3 p, fathom_vec3 s)
{
    fathom_vec3 q;
    fathom_vec3 ratio;

    /* ratio = p / s */
    ratio.x = p.x / s.x;
    ratio.y = p.y / s.y;
    ratio.z = p.z / s.z;

    /* q = p - s * round(ratio) */
    q.x = p.x - s.x * (f32)((i32)(ratio.x + ((ratio.x >= 0.0f) ? 0.5f : -0.5f)));
    q.y = p.y - s.y * (f32)((i32)(ratio.y + ((ratio.y >= 0.0f) ? 0.5f : -0.5f)));
    q.z = p.z - s.z * (f32)((i32)(ratio.z + ((ratio.z >= 0.0f) ? 0.5f : -0.5f)));

    return q;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_sdf_op_repeat_limited(fathom_vec3 p, f32 s, fathom_vec3 l)
{
    fathom_vec3 q;
    f32 rx, ry, rz;

    /* ratio = p / s */
    rx = p.x / s;
    ry = p.y / s;
    rz = p.z / s;

    /* q = p - s * clamp(round(p / s), -l, l) */
    q.x = p.x - s * fathom_clampf(fathom_roundf(rx), -l.x, l.x);
    q.y = p.y - s * fathom_clampf(fathom_roundf(ry), -l.y, l.y);
    q.z = p.z - s * fathom_clampf(fathom_roundf(rz), -l.z, l.z);

    return q;
}

/* #############################################################################
 * # [SECTION] Signed Distance Axis Aligned Bounding Boxes (AABB)
 * #############################################################################
 */
typedef struct fathom_sdf_aabb
{
    fathom_vec3 min;
    fathom_vec3 max;

} fathom_sdf_aabb;

FATHOM_API FATHOM_INLINE f32 fathom_sdf_aabb_distance(fathom_vec3 p, fathom_sdf_aabb *box)
{
    f32 dx = 0.0f;
    f32 dy = 0.0f;
    f32 dz = 0.0f;

    if (p.x < box->min.x)
    {
        dx = box->min.x - p.x;
    }
    else if (p.x > box->max.x)
    {
        dx = p.x - box->max.x;
    }

    if (p.y < box->min.y)
    {
        dy = box->min.y - p.y;
    }
    else if (p.y > box->max.y)
    {
        dy = p.y - box->max.y;
    }

    if (p.z < box->min.z)
    {
        dz = box->min.z - p.z;
    }
    else if (p.z > box->max.z)
    {
        dz = p.z - box->max.z;
    }

    return dx * dx + dy * dy + dz * dz;
}

FATHOM_API FATHOM_INLINE fathom_sdf_aabb fathom_sdf_aabb_sphere(fathom_vec3 center, f32 radius)
{
    fathom_sdf_aabb b;

    b.min.x = center.x - radius;
    b.min.y = center.y - radius;
    b.min.z = center.z - radius;

    b.max.x = center.x + radius;
    b.max.y = center.y + radius;
    b.max.z = center.z + radius;

    return b;
}

#endif /* FATHOM_MATH_SDF_H */