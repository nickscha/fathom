#ifndef FATHOM_SPARSE_GRID_H
#define FATHOM_SPARSE_GRID_H

#include "fathom_math_linear_algebra.h"

FATHOM_API FATHOM_INLINE u32 fathom_ceil_div_u32(u32 a, u32 b)
{
    return (a + b - 1) / b;
}

FATHOM_API FATHOM_INLINE u32 fathom_cuberoot_ceil_u32(u32 x)
{
    u32 r = 0;

    while (r * r * r < x)
    {
        r++;
    }

    return r;
}

#define FATHOM_BRICK_SIZE 8
#define FATHOM_BRICK_MAP_BYTES sizeof(u16) /* 2 = R16UI */
#define FATHOM_ATLAS_BYTES sizeof(u8)      /* 1 = R8 */

typedef struct fathom_sparse_grid_layout
{
    u32 grid_dimension;              /* grid cells per axis */
    u32 brick_resolution;            /* bricks per axis */
    u32 brick_map_bytes;             /* brick map */
    u32 atlas_resolution_worst_case; /* voxels per axis (worst case) */
    u32 atlas_bytes_worst_case;      /* atlas (worst case)*/

} fathom_sparse_grid_layout;

FATHOM_API u8 fathom_sparse_grid_layout_initialize(fathom_sparse_grid_layout *sparse_grid_layout, u32 grid_dimension)
{
    u32 brick_resolution = fathom_ceil_div_u32(grid_dimension, FATHOM_BRICK_SIZE);
    u32 total_bricks = brick_resolution * brick_resolution * brick_resolution;
    u32 atlas_bricks_per_axis = fathom_cuberoot_ceil_u32(total_bricks);
    u32 atlas_resolution_worst_case = atlas_bricks_per_axis * FATHOM_BRICK_SIZE;
    u32 brick_map_bytes = total_bricks * FATHOM_BRICK_MAP_BYTES;
    u32 atlas_bytes_worst_case = atlas_resolution_worst_case * atlas_resolution_worst_case * atlas_resolution_worst_case * FATHOM_ATLAS_BYTES;

    if (!sparse_grid_layout)
    {
        return 0;
    }

    sparse_grid_layout->grid_dimension = grid_dimension;
    sparse_grid_layout->brick_resolution = brick_resolution;
    sparse_grid_layout->atlas_resolution_worst_case = atlas_resolution_worst_case;
    sparse_grid_layout->brick_map_bytes = brick_map_bytes;
    sparse_grid_layout->atlas_bytes_worst_case = atlas_bytes_worst_case;

    return 1;
}

typedef struct fathom_sparse_grid
{
    fathom_vec3 grid_center;
    f32 grid_cell_size;

    u32 grid_dimension;
    u32 brick_resolution;
    u32 brick_map_bytes_used;

    u32 allocated_brick_count; /* real bricks used */
    u32 atlas_resolution;      /* real resolution */
    u32 atlas_bytes_used;

    u16 *brick_map;
    u8 *atlas_data;

} fathom_sparse_grid;

typedef f32 (*fathom_distance_function)(fathom_vec3 position);

/*
glTexImage3D(GL_TEXTURE_3D, 0, GL_R16UI,
             grid->brick_resolution,
             grid->brick_resolution,
             grid->brick_resolution,
             0, GL_RED_INTEGER, GL_UNSIGNED_SHORT,
             grid->brick_map);

glTexImage3D(GL_TEXTURE_3D, 0, GL_R8,
             grid->atlas_resolution,
             grid->atlas_resolution,
             grid->atlas_resolution,
             0, GL_RED, GL_UNSIGNED_BYTE,
             grid->atlas_data);
*/
FATHOM_API u8 fathom_sparse_grid_build(fathom_sparse_grid *grid, fathom_sparse_grid_layout *grid_layout, void *memory, fathom_vec3 grid_center, f32 grid_cell_size, fathom_distance_function distance_function)
{

    /* Check input */
    if (!grid || !grid_layout || !memory || !distance_function || grid_layout->grid_dimension == 0 || grid_cell_size <= 0.0f)
    {
        return 0;
    }

    grid->grid_center = grid_center;
    grid->grid_cell_size = grid_cell_size;
    grid->grid_dimension = grid_layout->grid_dimension;
    grid->brick_resolution = grid_layout->brick_resolution;
    grid->brick_map_bytes_used = grid_layout->brick_map_bytes;

    /* Assign memory */
    {
        u8 *ptr = (u8 *)memory;
        grid->brick_map = (u16 *)ptr;
        ptr += grid_layout->brick_map_bytes;

        grid->atlas_data = ptr;
        ptr += grid_layout->atlas_bytes_worst_case;

        if (!grid->brick_map || !grid->atlas_data)
        {
            return 0;
        }
    }

    /* Process Bricks */
    {
        u32 brick_x;
        u32 brick_y;
        u32 brick_z;

        u32 brick_resolution = grid_layout->brick_resolution;
        u32 grid_dimension = grid_layout->grid_dimension;
        f32 half_extent = ((f32)grid_dimension * grid_cell_size) * 0.5f;

        u32 atlas_brick_counter = 1; /* 0 reserved for empty */
        u32 brick_index_linear = 0;

        f32 brick_world_size = FATHOM_BRICK_SIZE * grid_cell_size;

        /* maximum useful distance inside a brick */
        f32 brick_diag = brick_world_size * 1.7320508f; /* sqrt(3) */

        f32 world_origin_x = grid_center.x - half_extent + 0.5f * grid_cell_size;
        f32 world_origin_y = grid_center.y - half_extent + 0.5f * grid_cell_size;
        f32 world_origin_z = grid_center.z - half_extent + 0.5f * grid_cell_size;

        u8 brick_voxels[FATHOM_BRICK_SIZE * FATHOM_BRICK_SIZE * FATHOM_BRICK_SIZE];

        u32 atlas_bricks_per_axis = grid_layout->atlas_resolution_worst_case / FATHOM_BRICK_SIZE;
        u32 atlas_resolution_worst_case = grid_layout->atlas_resolution_worst_case;

        for (brick_z = 0; brick_z < brick_resolution; ++brick_z)
        {

            for (brick_y = 0; brick_y < brick_resolution; ++brick_y)
            {

                for (brick_x = 0; brick_x < brick_resolution; ++brick_x)
                {
                    u32 base_voxel_z = brick_z * FATHOM_BRICK_SIZE;
                    u32 base_voxel_y = brick_y * FATHOM_BRICK_SIZE;
                    u32 base_voxel_x = brick_x * FATHOM_BRICK_SIZE;

                    f32 brick_base_x = world_origin_x + (f32)base_voxel_x * grid_cell_size;
                    f32 brick_base_y = world_origin_y + (f32)base_voxel_y * grid_cell_size;
                    f32 brick_base_z = world_origin_z + (f32)base_voxel_z * grid_cell_size;

                    u32 local_index = 0;
                    u8 has_positive = 0;
                    u8 has_negative = 0;

                    u32 voxel_x;
                    u32 voxel_y;
                    u32 voxel_z;

                    f32 z_pos = brick_base_z;

                    f32 brick_center_x = brick_base_x + 0.5f * brick_world_size;
                    f32 brick_center_y = brick_base_y + 0.5f * brick_world_size;
                    f32 brick_center_z = brick_base_z + 0.5f * brick_world_size;
                    f32 brick_center_distance = distance_function(fathom_vec3_init(brick_center_x, brick_center_y, brick_center_z));

                    if (brick_center_distance >= brick_diag * 0.5f)
                    {
                        /* Brick fully outside surface, skip it */
                        grid->brick_map[brick_index_linear++] = 0;
                        continue;
                    }

                    /* skip fully inside bricks */
                    if (brick_center_distance <= -brick_diag * 0.5f)
                    {
                        grid->brick_map[brick_index_linear++] = 0;
                        continue;
                    }

                    for (voxel_z = 0; voxel_z < FATHOM_BRICK_SIZE; ++voxel_z)
                    {
                        f32 y_pos = brick_base_y;

                        for (voxel_y = 0; voxel_y < FATHOM_BRICK_SIZE; ++voxel_y)
                        {
                            f32 x_pos = brick_base_x;

                            for (voxel_x = 0; voxel_x < FATHOM_BRICK_SIZE; ++voxel_x)
                            {
                                fathom_vec3 world_pos = fathom_vec3_init(x_pos, y_pos, z_pos);

                                f32 d = distance_function(world_pos);
                                f32 normalized;

                                if (d >= 0.0f)
                                {
                                    has_positive = 1;
                                }
                                else
                                {
                                    has_negative = 1;
                                }

                                /* normalize to [0,255] */
                                normalized = d / brick_diag;

                                if (normalized < -1.0f)
                                {
                                    normalized = -1.0f;
                                }
                                if (normalized > 1.0f)
                                {
                                    normalized = 1.0f;
                                }

                                /* map -1..1 → 0..255 */
                                normalized = (normalized * 0.5f + 0.5f) * 255.0f;

                                brick_voxels[local_index++] = (u8)normalized;

                                x_pos += grid_cell_size;
                            }
                            y_pos += grid_cell_size;
                        }

                        z_pos += grid_cell_size;
                    }

                    if (!(has_positive && has_negative))
                    {
                        grid->brick_map[brick_index_linear++] = 0;
                    }
                    else
                    {
                        u32 atlas_index;

                        if (atlas_brick_counter >
                            (atlas_resolution_worst_case / FATHOM_BRICK_SIZE) *
                                (atlas_resolution_worst_case / FATHOM_BRICK_SIZE) *
                                (atlas_resolution_worst_case / FATHOM_BRICK_SIZE))
                        {
                            /* atlas overflow (should never happen in worst-case allocation) */
                            return 0;
                        }

                        atlas_index = atlas_brick_counter++;

                        grid->brick_map[brick_index_linear++] = (u16)atlas_index;

                        /* compute atlas brick position */
                        {
                            u32 abx = atlas_index % atlas_bricks_per_axis;
                            u32 aby = (atlas_index / atlas_bricks_per_axis) % atlas_bricks_per_axis;
                            u32 abz = atlas_index / (atlas_bricks_per_axis * atlas_bricks_per_axis);

                            u32 atlas_base_x = abx * FATHOM_BRICK_SIZE;
                            u32 atlas_base_y = aby * FATHOM_BRICK_SIZE;
                            u32 atlas_base_z = abz * FATHOM_BRICK_SIZE;

                            /* write brick into atlas */
                            {
                                u32 i = 0;
                                for (voxel_z = 0; voxel_z < FATHOM_BRICK_SIZE; ++voxel_z)
                                {
                                    for (voxel_y = 0; voxel_y < FATHOM_BRICK_SIZE; ++voxel_y)
                                    {
                                        for (voxel_x = 0; voxel_x < FATHOM_BRICK_SIZE; ++voxel_x)
                                        {
                                            u32 ax = atlas_base_x + voxel_x;
                                            u32 ay = atlas_base_y + voxel_y;
                                            u32 az = atlas_base_z + voxel_z;
                                            u32 atlas_linear = ax + ay * atlas_resolution_worst_case + az * atlas_resolution_worst_case * atlas_resolution_worst_case;

                                            grid->atlas_data[atlas_linear] = brick_voxels[i++];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        grid->allocated_brick_count = atlas_brick_counter - 1;

        if (grid->allocated_brick_count == 0)
        {
            grid->atlas_resolution = 0;
        }
        else
        {
            u32 atlas_bricks_per_axis = fathom_cuberoot_ceil_u32(grid->allocated_brick_count);
            grid->atlas_resolution = atlas_bricks_per_axis * FATHOM_BRICK_SIZE;
            grid->atlas_bytes_used = grid->atlas_resolution * grid->atlas_resolution * grid->atlas_resolution;
        }
    }

    return 1;
}

#endif /* FATHOM_SPARSE_GRID_H */