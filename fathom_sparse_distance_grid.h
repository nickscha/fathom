#ifndef FATHOM_SPARSE_DISTANCE_GRID_H
#define FATHOM_SPARSE_DISTANCE_GRID_H

#include "fathom_math_linear_algebra.h"

FATHOM_API void *fathom_memcpy(void *dest, void *src, u32 count)
{
    s8 *dest8 = (s8 *)dest;
    s8 *src8 = (s8 *)src;

    while (count--)
    {
        *dest8++ = *src8++;
    }

    return dest;
}

/* #############################################################################
 * # [SECTION] Sparse Distance Grid Setup
 * #############################################################################
 */
#define FATHOM_BRICK_SIZE 8
#define FATHOM_BRICK_APRON 1
#define FATHOM_PHYSICAL_BRICK_SIZE (FATHOM_BRICK_SIZE + (2 * FATHOM_BRICK_APRON))                                        /* 10 */
#define FATHOM_BRICK_TOTAL_VOXELS (FATHOM_PHYSICAL_BRICK_SIZE * FATHOM_PHYSICAL_BRICK_SIZE * FATHOM_PHYSICAL_BRICK_SIZE) /* 1000 */

typedef f32 (*fathom_distance_function)(fathom_vec3 position, void *user_data);

typedef struct fathom_sparse_distance_grid
{
    fathom_vec3 start;    /* World space start */
    f32 cell_size;        /* The size of each cell */
    u32 grid_dim_bricks;  /* Grid Dimension for bricks */
    u32 atlas_dim_bricks; /* Grid Dimension for bricks: 16 = (atlas 16x16x16) + (brick 10×10×10=1,000 voxels ) = 4096 * 1000 bytes = 4mb*/

    u16 *brick_map;
    u8 *atlas_data;

    u32 brick_map_bytes;
    u32 atlas_bytes;

    f32 truncation_distance;

} fathom_sparse_distance_grid;

FATHOM_API u8 fathom_sparse_distance_grid_initialize(fathom_sparse_distance_grid *grid, u32 grid_cell_count, u32 atlas_bricks_per_side)
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

FATHOM_API u8 fathom_sparse_distance_grid_assign_memory(fathom_sparse_distance_grid *grid, void *memory)
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

/* Helper to calculate linear index in a 3D array */
FATHOM_API FATHOM_INLINE u32 fathom_sparse_distance_grid_get_index(u32 x, u32 y, u32 z, u32 width, u32 height)
{
    return x + (y * width) + (z * width * height);
}

FATHOM_API u8 fathom_sparse_distance_grid_calculate(
    fathom_sparse_distance_grid *grid,
    fathom_distance_function distance_function,
    void *user_data,
    fathom_vec3 grid_center,
    u32 grid_cell_count,
    f32 grid_cell_size)
{
    u32 bx, by, bz;
    u32 lx, ly, lz;
    u8 brick_temp_buffer[FATHOM_PHYSICAL_BRICK_SIZE][FATHOM_PHYSICAL_BRICK_SIZE][FATHOM_PHYSICAL_BRICK_SIZE];

    u32 atlas_capacity_bricks = grid->atlas_dim_bricks * grid->atlas_dim_bricks * grid->atlas_dim_bricks;
    u32 atlas_used_bricks = 0;

    if (!grid || !grid->brick_map || !grid->atlas_data)
    {
        return 0;
    }

    grid->cell_size = grid_cell_size;
    grid->start = fathom_vec3_subf(grid_center, (f32)grid_cell_count * grid_cell_size * 0.5f); /* size half offset */
    grid->truncation_distance = grid->cell_size * 4.0f;                                        /* 4 voxels of safe stepping */

    /* 1. Iterate over the coarse Meta-Grid (Bricks) */
    for (bz = 0; bz < grid->grid_dim_bricks; ++bz)
    {
        for (by = 0; by < grid->grid_dim_bricks; ++by)
        {
            for (bx = 0; bx < grid->grid_dim_bricks; ++bx)
            {
                u8 is_brick_useful = 0;
                u32 map_idx = fathom_sparse_distance_grid_get_index(bx, by, bz, grid->grid_dim_bricks, grid->grid_dim_bricks);

                fathom_vec3 brick_start_pos = fathom_vec3_init(
                    grid->start.x + (f32)(bx * FATHOM_BRICK_SIZE) * grid->cell_size,
                    grid->start.y + (f32)(by * FATHOM_BRICK_SIZE) * grid->cell_size,
                    grid->start.z + (f32)(bz * FATHOM_BRICK_SIZE) * grid->cell_size);

                fathom_vec3 brick_center = fathom_vec3_init(
                    brick_start_pos.x + (FATHOM_BRICK_SIZE * 0.5f) * grid->cell_size,
                    brick_start_pos.y + (FATHOM_BRICK_SIZE * 0.5f) * grid->cell_size,
                    brick_start_pos.z + (FATHOM_BRICK_SIZE * 0.5f) * grid->cell_size);

                f32 brick_radius = (FATHOM_PHYSICAL_BRICK_SIZE * 0.5f) * 1.732f * grid->cell_size;

                f32 center_dist = distance_function(brick_center, user_data);

                /* If the brick is completly empty fast skip */
                if (fathom_absf(center_dist) > (brick_radius + grid->truncation_distance))
                {
                    continue;
                }

                /* 2. Process the 10x10x10 voxels (Logical 8x8x8 + 1 voxel apron) */
                for (lz = 0; lz < FATHOM_PHYSICAL_BRICK_SIZE; ++lz)
                {
                    for (ly = 0; ly < FATHOM_PHYSICAL_BRICK_SIZE; ++ly)
                    {
                        for (lx = 0; lx < FATHOM_PHYSICAL_BRICK_SIZE; ++lx)
                        {

                            fathom_vec3 pos;
                            f32 dist, d;
                            u8 dist_quant;

                            /* Map physical loop [0..9] to logical voxel indices [-1..8] */
                            i32 logical_x = (i32)lx - FATHOM_BRICK_APRON;
                            i32 logical_y = (i32)ly - FATHOM_BRICK_APRON;
                            i32 logical_z = (i32)lz - FATHOM_BRICK_APRON;

                            /* Sample at the center of the logical voxel */
                            pos = fathom_vec3_init(
                                brick_start_pos.x + ((f32)logical_x + 0.5f) * grid->cell_size,
                                brick_start_pos.y + ((f32)logical_y + 0.5f) * grid->cell_size,
                                brick_start_pos.z + ((f32)logical_z + 0.5f) * grid->cell_size);

                            dist = distance_function(pos, user_data);

                            /* Clamp to widened truncation band */
                            d = fathom_clampf(dist / grid->truncation_distance, -1.0f, 1.0f);
                            dist_quant = (u8)((d * 0.5f + 0.5f) * 255.0f);

                            brick_temp_buffer[lz][ly][lx] = dist_quant;

                            /* If ANY voxel in the logical block OR apron is close to the surface, keep the brick */
                            if (fathom_absf(dist) <= grid->truncation_distance)
                            {
                                is_brick_useful = 1;
                            }
                        }
                    }
                }

                /* 3. Write to Atlas or Explicitly Clear Map */
                if (is_brick_useful)
                {
                    /* C89: Declare variables at top of block */
                    u32 current_brick_idx, atlas_bx, atlas_by, atlas_bz;
                    u32 origin_x, origin_y, origin_z, atlas_voxels;

                    if (atlas_used_bricks >= atlas_capacity_bricks)
                    {
                        return 0; /* Atlas Full */
                    }

                    current_brick_idx = atlas_used_bricks++;
                    grid->brick_map[map_idx] = (u16)(current_brick_idx + 1);

                    atlas_bx = current_brick_idx % grid->atlas_dim_bricks;
                    atlas_by = (current_brick_idx / grid->atlas_dim_bricks) % grid->atlas_dim_bricks;
                    atlas_bz = current_brick_idx / (grid->atlas_dim_bricks * grid->atlas_dim_bricks);

                    /* Origin calculation uses the new physical size */
                    origin_x = atlas_bx * FATHOM_PHYSICAL_BRICK_SIZE;
                    origin_y = atlas_by * FATHOM_PHYSICAL_BRICK_SIZE;
                    origin_z = atlas_bz * FATHOM_PHYSICAL_BRICK_SIZE;

                    atlas_voxels = grid->atlas_dim_bricks * FATHOM_PHYSICAL_BRICK_SIZE;

                    for (lz = 0; lz < FATHOM_PHYSICAL_BRICK_SIZE; ++lz)
                    {
                        for (ly = 0; ly < FATHOM_PHYSICAL_BRICK_SIZE; ++ly)
                        {
                            u32 target_index = fathom_sparse_distance_grid_get_index(origin_x, origin_y + ly, origin_z + lz, atlas_voxels, atlas_voxels);

                            fathom_memcpy(&grid->atlas_data[target_index], &brick_temp_buffer[lz][ly][0], FATHOM_PHYSICAL_BRICK_SIZE);
                        }
                    }
                }
            }
        }
    }

    return 1;
}

#endif /* FATHOM_SPARSE_DISTANCE_GRID_H */