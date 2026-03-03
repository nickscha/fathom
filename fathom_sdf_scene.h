#ifndef FATHOM_SDF_SCENE_H
#define FATHOM_SDF_SCENE_H

#include "fathom_types.h"
#include "fathom_math_sdf.h"
#include "fathom_sparse_grid.h"

/* #############################################################################
 * # [SECTION] SDF Scene
 * #############################################################################
 */
typedef enum fathom_sdf_primitive_id
{
    FATHOM_SDF_PRIMITIVE_SPHERE = 0,
    FATHOM_SDF_PRIMITIVE_BOX,
    FATHOM_SDF_PRIMITIVE_BOX_FRAME,
    FATHOM_SDF_PRIMITIVE_ELLIPSOID,
    FATHOM_SDF_PRIMITIVE_OCTAHEDRON,
    FATHOM_SDF_PRIMITIVE_COUNT

} fathom_sdf_primitive_id;

typedef struct fathom_sdf_primitive
{
    u8 primitive_id; /* fathom_sdf_primitive */
    u8 material_id;

    fathom_vec3 position;

    union
    {
        struct
        {
            f32 radius;
        } sphere;

        struct
        {
            fathom_vec3 base;
        } box;

        struct
        {
            fathom_vec3 base;
            f32 edge_thickness;
        } box_frame;

        struct
        {
            fathom_vec3 radius;
        } ellipsoid;

        struct
        {
            f32 scale;
        } octahedron;

    } attributes;

} fathom_sdf_primitive;

#define FATHOM_SDF_PRIMITIVE_COUNT 4
static fathom_sdf_primitive primitives[FATHOM_SDF_PRIMITIVE_COUNT];
static fathom_sdf_aabb sdf_scene_aabb;

FATHOM_API void fathom_sdf_scene_build(void)
{
    fathom_sdf_primitive sphere = {0};
    fathom_sdf_primitive box = {0};
    fathom_sdf_primitive ellipsoid = {0};
    fathom_sdf_primitive octahedron = {0};

    sphere.primitive_id = FATHOM_SDF_PRIMITIVE_SPHERE;
    sphere.material_id = 1;
    sphere.position = fathom_vec3_zero;
    sphere.attributes.sphere.radius = 0.5f;

    box.primitive_id = FATHOM_SDF_PRIMITIVE_BOX;
    box.material_id = 2;
    box.position = fathom_vec3_init(-0.5f, 0.5f, -0.5f);
    box.attributes.box.base = fathom_vec3_initf(0.25f);

    ellipsoid.primitive_id = FATHOM_SDF_PRIMITIVE_ELLIPSOID;
    ellipsoid.position = fathom_vec3_init(1.0f, 0.5f, -0.5f);
    ellipsoid.attributes.ellipsoid.radius = fathom_vec3_init(0.5f, 0.25f, 0.125f);

    octahedron.primitive_id = FATHOM_SDF_PRIMITIVE_OCTAHEDRON;
    octahedron.position = fathom_vec3_init(0.0f, 0.5f, -1.0f);
    octahedron.attributes.octahedron.scale = 0.25f;

    primitives[0] = sphere;
    primitives[1] = box;
    primitives[2] = ellipsoid;
    primitives[3] = octahedron;

    /* Set simple bounding box */
    sdf_scene_aabb.min = fathom_vec3_init(-2.0f, -1.0f, -2.0f);
    sdf_scene_aabb.max = fathom_vec3_init(2.0f, 2.0f, 2.0f);
}

FATHOM_API fathom_grid_data fathom_sdf_scene(fathom_vec3 position, void *user_data)
{
    f32 ground = position.y - (-0.25f);

    fathom_grid_data d;
    d.distance = ground;
    d.material = 0;

    (void)user_data;

    if (fathom_sdf_aabb_distance(position, &sdf_scene_aabb) <= d.distance)
    {
        f32 primitive_distance_total = 1e30f; /* very large start distance */
        u8 primitive_material_total = 0;

        u32 i;

        for (i = 0; i < FATHOM_SDF_PRIMITIVE_COUNT; ++i)
        {
            fathom_sdf_primitive primitive = primitives[i];
            fathom_vec3 primitive_pos = fathom_vec3_sub(position, primitive.position);
            f32 primitive_distance = 0.0f;

            switch (primitive.primitive_id)
            {
            case FATHOM_SDF_PRIMITIVE_SPHERE:
            {
                primitive_distance = fathom_sdf_sphere(primitive_pos, primitive.attributes.sphere.radius);
            }
            break;
            case FATHOM_SDF_PRIMITIVE_BOX:
            {
                primitive_distance = fathom_sdf_box(primitive_pos, primitive.attributes.box.base);
            }
            break;
            case FATHOM_SDF_PRIMITIVE_ELLIPSOID:
            {
                primitive_distance = fathom_sdf_ellipsoid(primitive_pos, primitive.attributes.ellipsoid.radius);
            }
            break;
            case FATHOM_SDF_PRIMITIVE_OCTAHEDRON:
            {
                primitive_distance = fathom_sdf_octahedron(primitive_pos, primitive.attributes.octahedron.scale);
            }
            break;
            default:
                break;
            }

            primitive_distance_total = fathom_sdf_op_union_smooth(primitive_distance_total, primitive_distance, 0.4f);
        }

        primitive_distance_total = fathom_sdf_op_union_smooth(ground, primitive_distance_total, 0.4f);

        d.distance = primitive_distance_total;
        d.material = primitive_material_total;
    }

    return d;
}

/* old keep for reference */
FATHOM_API FATHOM_INLINE fathom_grid_data fathom_sdf_scene_original(fathom_vec3 position, void *user_data)
{
    static u8 sdf_scene_initialized;
    static fathom_sdf_aabb sdf_scene_aabb;

    f32 ground = position.y - (-0.25f);

    fathom_grid_data d;
    d.material = 0;

    if (!sdf_scene_initialized)
    {
        sdf_scene_aabb.min = fathom_vec3_init(-2.0f, -1.0f, -2.0f);
        sdf_scene_aabb.max = fathom_vec3_init(2.0f, 2.0f, 2.0f);

        sdf_scene_initialized = 1;
    }

    if (fathom_sdf_aabb_distance(position, &sdf_scene_aabb) <= d.distance)
    {

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
    }

    return d;
}

#endif /* FATHOM_SDF_SCENE_H */