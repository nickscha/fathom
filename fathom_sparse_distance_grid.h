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
#define FATHOM_BRICK_TOTAL_VOXELS (FATHOM_BRICK_SIZE * FATHOM_BRICK_SIZE * FATHOM_BRICK_SIZE)
#define FATHOM_ATLAS_WIDTH_IN_BRICKS 16 /* The Atlas is 16 * 8 = 128 voxels wide */
#define FATHOM_ATLAS_HEIGHT_IN_BRICKS 16
#define FATHOM_ATLAS_DEPTH_IN_BRICKS 16

typedef f32 (*fathom_distance_function)(fathom_vec3 position);

typedef struct fathom_sparse_distance_grid
{
    fathom_vec3 center;
    u32 cell_count;    /* Total resolution (e.g., 64) */
    fathom_vec3 start; /* World space start */
    f32 cell_size;
    f32 cell_space_diagonal;
    f32 size_half;

    /* Grid Dimensions in Bricks */
    u32 grid_dim_bricks_x;
    u32 grid_dim_bricks_y;
    u32 grid_dim_bricks_z;

    u16 *brick_map;
    u8 *atlas_data;
    u32 atlas_capacity_bricks;
    u32 atlas_used_bricks;

    u32 brick_map_bytes;
    u32 atlas_bytes;

    fathom_distance_function distance_function;

} fathom_sparse_distance_grid;

/* Helper to calculate linear index in a 3D array */
FATHOM_API FATHOM_INLINE u32 fathom_get_index(u32 x, u32 y, u32 z, u32 width, u32 height)
{
    return x + (y * width) + (z * width * height);
}

FATHOM_API u8 fathom_sparse_distance_grid_initialize(fathom_sparse_distance_grid *grid, fathom_vec3 grid_center, u32 cell_count, f32 grid_cell_size, fathom_distance_function distance_function)
{
    u32 brick_map_count;
    u32 atlas_byte_size;

    if (!grid || !distance_function || cell_count == 0 || grid_cell_size <= 0.0f || cell_count % FATHOM_BRICK_SIZE != 0)
    {
        return 0;
    }

    grid->center = grid_center;
    grid->cell_count = cell_count;
    grid->cell_size = grid_cell_size;
    grid->cell_space_diagonal = grid_cell_size * 1.7320508f; /* sqrt(3) */
    grid->size_half = (f32)cell_count * grid_cell_size * 0.5f;
    grid->start = fathom_vec3_subf(grid_center, grid->size_half);
    grid->distance_function = distance_function;

    /* Calculate Brick Grid Dimensions */
    grid->grid_dim_bricks_x = cell_count / FATHOM_BRICK_SIZE;
    grid->grid_dim_bricks_y = cell_count / FATHOM_BRICK_SIZE;
    grid->grid_dim_bricks_z = cell_count / FATHOM_BRICK_SIZE;

    /* Allocate Brick Map (The Indirection Grid) */
    brick_map_count = grid->grid_dim_bricks_x * grid->grid_dim_bricks_y * grid->grid_dim_bricks_z;
    grid->brick_map_bytes = brick_map_count * sizeof(u16);

    /* Allocate Atlas (The 3D Data Texture Buffer) */
    /* Size = (W*8) * (H*8) * (D*8) bytes */
    grid->atlas_capacity_bricks = FATHOM_ATLAS_WIDTH_IN_BRICKS * FATHOM_ATLAS_HEIGHT_IN_BRICKS * FATHOM_ATLAS_DEPTH_IN_BRICKS;
    atlas_byte_size = grid->atlas_capacity_bricks * FATHOM_BRICK_TOTAL_VOXELS;

    grid->atlas_bytes = atlas_byte_size * sizeof(u8);
    grid->atlas_used_bricks = 0;

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

FATHOM_API u8 fathom_sparse_distance_grid_calculate(fathom_sparse_distance_grid *grid)
{
    u32 bx, by, bz; /* Brick coordinates */
    u32 lx, ly, lz; /* Local brick coordinates (0-7) */

    /* Temp buffer to hold one brick's data before we decide to commit it */
    u8 brick_temp_buffer[FATHOM_BRICK_SIZE][FATHOM_BRICK_SIZE][FATHOM_BRICK_SIZE];

    if (!grid || !grid->brick_map || !grid->atlas_data)
    {
        return 0;
    }

    /* 1. Iterate over the coarse Meta-Grid (Bricks) */
    for (bz = 0; bz < grid->grid_dim_bricks_z; ++bz)
    {
        for (by = 0; by < grid->grid_dim_bricks_y; ++by)
        {
            for (bx = 0; bx < grid->grid_dim_bricks_x; ++bx)
            {

                u8 is_brick_useful = 0;
                fathom_vec3 brick_start_pos;

                /* Calculate world position of this brick's corner */
                brick_start_pos.x = grid->start.x + (f32)(bx * FATHOM_BRICK_SIZE) * grid->cell_size;
                brick_start_pos.y = grid->start.y + (f32)(by * FATHOM_BRICK_SIZE) * grid->cell_size;
                brick_start_pos.z = grid->start.z + (f32)(bz * FATHOM_BRICK_SIZE) * grid->cell_size;

                /* 2. Process the 8x8x8 voxels inside this brick */
                for (lz = 0; lz < FATHOM_BRICK_SIZE; ++lz)
                {
                    for (ly = 0; ly < FATHOM_BRICK_SIZE; ++ly)
                    {
                        for (lx = 0; lx < FATHOM_BRICK_SIZE; ++lx)
                        {
                            fathom_vec3 pos = fathom_vec3_init(
                                brick_start_pos.x + (f32)lx * grid->cell_size,
                                brick_start_pos.y + (f32)ly * grid->cell_size,
                                brick_start_pos.z + (f32)lz * grid->cell_size);

                            f32 dist = grid->distance_function(pos);

                            u8 dist_quant = fathom_quantizef(dist, grid->cell_space_diagonal);

                            brick_temp_buffer[lz][ly][lx] = dist_quant;

                            /* Check if this voxel is "interesting" (inside the narrow band) */
                            if (dist <= grid->cell_space_diagonal)
                            {
                                is_brick_useful = 1;
                            }
                        }
                    }
                }

                /* 3. If the brick contains surface data, write to Atlas */
                if (is_brick_useful)
                {
                    if (grid->atlas_used_bricks >= grid->atlas_capacity_bricks)
                    {
                        /* Atlas Full */
                        return 0;
                    }

                    /* Allocate next slot in Atlas */
                    u32 current_brick_idx = grid->atlas_used_bricks++;

                    /* Convert linear brick index to 3D Atlas Brick Coordinates */
                    /* Note: This depends on how you layout the Atlas texture.
                       Here we fill it linearly: X then Y then Z */
                    u32 atlas_bx = current_brick_idx % FATHOM_ATLAS_WIDTH_IN_BRICKS;
                    u32 atlas_by = (current_brick_idx / FATHOM_ATLAS_WIDTH_IN_BRICKS) % FATHOM_ATLAS_HEIGHT_IN_BRICKS;
                    u32 atlas_bz = current_brick_idx / (FATHOM_ATLAS_WIDTH_IN_BRICKS * FATHOM_ATLAS_HEIGHT_IN_BRICKS);

                    /* Update Brick Map */
                    u32 map_idx = fathom_get_index(bx, by, bz, grid->grid_dim_bricks_x, grid->grid_dim_bricks_y);
                    u16 stored_index = (u16)(current_brick_idx + 1);
                    grid->brick_map[map_idx] = stored_index;

                    {
                        /* Origin in the Atlas Texture (in voxels) */
                        u32 origin_x = atlas_bx * FATHOM_BRICK_SIZE;
                        u32 origin_y = atlas_by * FATHOM_BRICK_SIZE;
                        u32 origin_z = atlas_bz * FATHOM_BRICK_SIZE;

                        u32 atlas_width_voxels = FATHOM_ATLAS_WIDTH_IN_BRICKS * FATHOM_BRICK_SIZE;
                        u32 atlas_height_voxels = FATHOM_ATLAS_HEIGHT_IN_BRICKS * FATHOM_BRICK_SIZE;

                        for (lz = 0; lz < FATHOM_BRICK_SIZE; ++lz)
                        {
                            for (ly = 0; ly < FATHOM_BRICK_SIZE; ++ly)
                            {
                                /* We can copy a whole row (X-axis) at once */
                                u32 target_index = fathom_get_index(
                                    origin_x,
                                    origin_y + ly,
                                    origin_z + lz,
                                    atlas_width_voxels,
                                    atlas_height_voxels);

                                fathom_memcpy(&grid->atlas_data[target_index], &brick_temp_buffer[lz][ly][0], FATHOM_BRICK_SIZE);
                            }
                        }
                    }
                }
            }
        }
    }

    return 1;
}

#endif /* FATHOM_SPARSE_DISTANCE_GRID_H */