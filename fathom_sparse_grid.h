#ifndef FATHOM_SPARSE_GRID_H
#define FATHOM_SPARSE_GRID_H

#include "fathom_math_linear_algebra.h"

/* #############################################################################
 * # [SECTION] Sparse Distance Grid Setup
 * #############################################################################
 */
#define FATHOM_BRICK_SIZE 8
#define FATHOM_BRICK_APRON 1
#define FATHOM_PHYSICAL_BRICK_SIZE (FATHOM_BRICK_SIZE + (2 * FATHOM_BRICK_APRON))                                        /* 10 */
#define FATHOM_BRICK_TOTAL_VOXELS (FATHOM_PHYSICAL_BRICK_SIZE * FATHOM_PHYSICAL_BRICK_SIZE * FATHOM_PHYSICAL_BRICK_SIZE) /* 1000 */

typedef f32 (*fathom_distance_function)(fathom_vec3 position, void *user_data);

typedef struct fathom_sparse_grid
{
    fathom_vec3 start;       /* World space start */
    f32 cell_size;           /* The size of each cell */
    u32 grid_dim_bricks;     /* Grid Dimension for bricks */
    u32 atlas_dim_bricks;    /* Grid Dimension for bricks: 16 = (atlas 16x16x16) + (brick 10×10×10=1,000 voxels ) = 4096 * 1000 bytes = 4mb*/
    f32 truncation_distance; /* Safe voxel stepping (e.g. grid->cell_size * 4.0f) */

    u32 brick_map_bytes;
    u32 atlas_bytes;

    u16 *brick_map;
    u8 *atlas_data;

} fathom_sparse_grid;

FATHOM_API u8 fathom_sparse_grid_initialize(fathom_sparse_grid *grid, u32 grid_cell_count, u32 atlas_bricks_per_side)
{
    if (!grid || grid_cell_count == 0 || grid_cell_count % FATHOM_BRICK_SIZE != 0)
    {
        return 0;
    }

    grid->grid_dim_bricks = grid_cell_count / FATHOM_BRICK_SIZE;
    grid->atlas_dim_bricks = atlas_bricks_per_side;
    grid->brick_map_bytes = grid->grid_dim_bricks * grid->grid_dim_bricks * grid->grid_dim_bricks * sizeof(u16);
    grid->atlas_bytes = grid->atlas_dim_bricks * grid->atlas_dim_bricks * grid->atlas_dim_bricks * FATHOM_BRICK_TOTAL_VOXELS * sizeof(u8);

    return 1;
}

FATHOM_API u8 fathom_sparse_grid_assign_memory(fathom_sparse_grid *grid, void *memory)
{
    u8 *ptr = (u8 *)memory;

    if (!grid || !memory || grid->brick_map_bytes == 0 || grid->atlas_bytes == 0)
    {
        return 0;
    }

    grid->brick_map = (u16 *)ptr;
    ptr += grid->brick_map_bytes;

    grid->atlas_data = ptr;
    ptr += grid->atlas_bytes;

    return 1;
}

FATHOM_API u8 fathom_sparse_grid_calculate(
    fathom_sparse_grid *grid,
    fathom_distance_function distance_function,
    void *user_data,
    fathom_vec3 grid_center,
    u32 grid_cell_count,
    f32 grid_cell_size)
{
    static u8 brick_temp_buffer[FATHOM_PHYSICAL_BRICK_SIZE][FATHOM_PHYSICAL_BRICK_SIZE][FATHOM_PHYSICAL_BRICK_SIZE];

    u32 bx, by, bz;

    u32 atlas_capacity_bricks;
    u32 atlas_used_bricks = 0;
    u32 atlas_voxels;
    u32 atlas_slice_size;
    u32 brick_map_index = 0;
    f32 brick_radius;
    f32 inverse_truncation_distance;
    f32 quant_scale;
    f32 apron_offset;
    f32 cull_threshold;

    if (!grid || !grid->brick_map || !grid->atlas_data)
    {
        return 0;
    }

    atlas_capacity_bricks = grid->atlas_dim_bricks * grid->atlas_dim_bricks * grid->atlas_dim_bricks;
    atlas_voxels = grid->atlas_dim_bricks * FATHOM_PHYSICAL_BRICK_SIZE;
    atlas_slice_size = atlas_voxels * atlas_voxels;

    grid->cell_size = grid_cell_size;
    grid->start = fathom_vec3_subf(grid_center, (f32)grid_cell_count * grid_cell_size * 0.5f);
    grid->truncation_distance = grid->cell_size * 4.0f;

    brick_radius = (FATHOM_PHYSICAL_BRICK_SIZE * 0.5f) * 1.7320508f * grid->cell_size;
    inverse_truncation_distance = 1.0f / grid->truncation_distance;
    quant_scale = 127.5f * inverse_truncation_distance;
    apron_offset = (0.5f - (f32)FATHOM_BRICK_APRON) * grid->cell_size;
    cull_threshold = brick_radius + grid->truncation_distance;

    /* 1. Iterate over the coarse Meta-Grid (Bricks) */
    for (bz = 0; bz < grid->grid_dim_bricks; ++bz)
    {
        for (by = 0; by < grid->grid_dim_bricks; ++by)
        {
            for (bx = 0; bx < grid->grid_dim_bricks; ++bx, ++brick_map_index)
            {
                u8 is_brick_useful = 0;

                fathom_vec3 brick_start_pos = fathom_vec3_init(
                    grid->start.x + (f32)(bx * FATHOM_BRICK_SIZE) * grid->cell_size,
                    grid->start.y + (f32)(by * FATHOM_BRICK_SIZE) * grid->cell_size,
                    grid->start.z + (f32)(bz * FATHOM_BRICK_SIZE) * grid->cell_size);

                fathom_vec3 brick_center_pos = fathom_vec3_addf(brick_start_pos, (FATHOM_BRICK_SIZE * 0.5f) * grid->cell_size);

                f32 brick_center_distance = distance_function(brick_center_pos, user_data);

                if (fathom_absf(brick_center_distance) > cull_threshold)
                {
                    grid->brick_map[brick_map_index] = (brick_center_distance > 0.0f) ? 0 : 0xFFFF; /* air = 0, solid = 0xFFFF */
                    continue;
                }

                /* 2. Process the 10x10x10 voxels */
                {
                    u32 lx, ly, lz;

                    f32 base_x = brick_start_pos.x + apron_offset;
                    f32 base_y = brick_start_pos.y + apron_offset;
                    f32 base_z = brick_start_pos.z + apron_offset;

                    f32 pz = base_z;

                    for (lz = 0; lz < FATHOM_PHYSICAL_BRICK_SIZE; ++lz, pz += grid->cell_size)
                    {
                        f32 py = base_y;

                        for (ly = 0; ly < FATHOM_PHYSICAL_BRICK_SIZE; ++ly, py += grid->cell_size)
                        {
                            f32 px = base_x;

                            for (lx = 0; lx < FATHOM_PHYSICAL_BRICK_SIZE; ++lx, px += grid->cell_size)
                            {
                                f32 distance_scaled = distance_function(fathom_vec3_init(px, py, pz), user_data) * quant_scale + 127.5f;

                                is_brick_useful |= (distance_scaled >= 0.0f && distance_scaled <= 255.0f);

                                distance_scaled = distance_scaled < 0.0f ? 0.0f : distance_scaled;
                                distance_scaled = distance_scaled > 255.0f ? 255.0f : distance_scaled;

                                brick_temp_buffer[lz][ly][lx] = (u8)distance_scaled;
                            }
                        }
                    }
                }

                /* 3. Write to Atlas */
                if (is_brick_useful)
                {
                    u32 current_brick_idx, atlas_bx, atlas_by, atlas_bz;
                    u32 origin_x, origin_y, origin_z;
                    u32 base_atlas_index, lz, ly;

                    if (atlas_used_bricks >= atlas_capacity_bricks)
                    {
                        return 0;
                    }

                    current_brick_idx = atlas_used_bricks++;
                    grid->brick_map[brick_map_index] = (u16)(current_brick_idx + 1);

                    atlas_bx = current_brick_idx % grid->atlas_dim_bricks;
                    atlas_by = (current_brick_idx / grid->atlas_dim_bricks) % grid->atlas_dim_bricks;
                    atlas_bz = current_brick_idx / (grid->atlas_dim_bricks * grid->atlas_dim_bricks);

                    origin_x = atlas_bx * FATHOM_PHYSICAL_BRICK_SIZE;
                    origin_y = atlas_by * FATHOM_PHYSICAL_BRICK_SIZE;
                    origin_z = atlas_bz * FATHOM_PHYSICAL_BRICK_SIZE;

                    base_atlas_index = origin_x + (origin_y * atlas_voxels) + (origin_z * atlas_slice_size);

                    for (lz = 0; lz < FATHOM_PHYSICAL_BRICK_SIZE; ++lz)
                    {
                        u32 z_offset = base_atlas_index + (lz * atlas_slice_size);

                        for (ly = 0; ly < FATHOM_PHYSICAL_BRICK_SIZE; ++ly)
                        {
                            u32 atlas_index = z_offset + (ly * atlas_voxels);
                            u8 *dst = &grid->atlas_data[atlas_index];
                            u8 *src = &brick_temp_buffer[lz][ly][0];

                            dst[0] = src[0];
                            dst[1] = src[1];
                            dst[2] = src[2];
                            dst[3] = src[3];
                            dst[4] = src[4];
                            dst[5] = src[5];
                            dst[6] = src[6];
                            dst[7] = src[7];
                            dst[8] = src[8];
                            dst[9] = src[9];
                        }
                    }
                }
            }
        }
    }

    return 1;
}

#endif /* FATHOM_SPARSE_GRID_H */