#ifndef FATHOM_SDF_SCENE_H
#define FATHOM_SDF_SCENE_H

#include "fathom_types.h"
#include "fathom_math_sdf.h"
#include "fathom_sparse_grid.h"

/* #############################################################################
 * # [SECTION] SDF Scene
 * #############################################################################
 */
/* Simple Sphere SDF */
FATHOM_API fathom_grid_data fathom_sdf_scene(fathom_vec3 position, void *user_data)
{
    fathom_grid_data d;

    f32 sphere_radius = 0.5f;
    f32 sphere = fathom_sdf_sphere(position, sphere_radius);

    fathom_vec3 box_pos = fathom_vec3_sub(position, fathom_vec3_init(-0.5f, 0.5f, -0.5f));
    fathom_mat2x2 box_rot = fathom_mat2x2_rot2d(FATHOM_DEG_TO_RAD(45.0f));
    f32 box_scale = 1.0f;

    (void)user_data;

    fathom_vec2_mul_mat2x2(&box_pos.x, &box_pos.y, box_rot); /* rotate around z axis */
    fathom_vec2_mul_mat2x2(&box_pos.y, &box_pos.z, box_rot); /* rotate around x axis */

    {
        f32 box = fathom_sdf_box(fathom_vec3_divf(box_pos, box_scale), fathom_vec3_init(0.25f, 0.25f, 0.25f)) * box_scale;

        fathom_vec3 ellipsoid_pos = fathom_vec3_sub(position, fathom_vec3_init(1.0f, 0.5f, -0.5f));
        f32 ellipsoid = fathom_sdf_ellipsoid(ellipsoid_pos, fathom_vec3_init(0.5f, 0.25f, 0.125f));

        fathom_vec3 box_frame_pos = fathom_vec3_sub(position, fathom_vec3_init(0.25f, 0.5f, -1.0f));
        fathom_vec3 box_frame_pos_repeat = fathom_sdf_op_repeat_limited(box_frame_pos, 1.0f, fathom_vec3_init(1.0f, 1.0f, 0.0f));
        f32 box_frame = fathom_sdf_box_frame(box_frame_pos_repeat, fathom_vec3_init(0.25f, 0.25f, 0.25f), 0.025f);

        f32 octahedron = fathom_sdf_octahedron(fathom_vec3_sub(position, fathom_vec3_init(0.0f, 0.25f, 0.35f)), 0.25f);

        f32 ground = position.y - (-0.25f);

        d.distance = fathom_sdf_op_union_smooth(ground, fathom_sdf_op_union_smooth(fathom_sdf_op_union_smooth(fathom_sdf_op_union_smooth(fathom_sdf_op_subtract(octahedron, sphere), box, 0.4f), ellipsoid, 0.2f), box_frame, 0.1f), 0.6f);
        d.material = 0;

        if (sphere < box && sphere < ground)
        {
            d.material = 1;
        }
        else if (box < sphere && box < ground)
        {
            d.material = 2;
        }
    }

    return d;
}

#endif /* FATHOM_SDF_SCENE_H */