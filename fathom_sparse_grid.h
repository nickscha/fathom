#ifndef FATHOM_SPARSE_GRID_H
#define FATHOM_SPARSE_GRID_H

#include "fathom_math_linear_algebra.h"

/* #############################################################################
 * # [SECTION] Sparse Grid Setup
 * #############################################################################
 */
#define FATHOM_BRICK_SIZE 8
#define FATHOM_BRICK_APRON 1
#define FATHOM_PHYSICAL_BRICK_SIZE (FATHOM_BRICK_SIZE + (2 * FATHOM_BRICK_APRON))                                        /* 10 */
#define FATHOM_BRICK_TOTAL_VOXELS (FATHOM_PHYSICAL_BRICK_SIZE * FATHOM_PHYSICAL_BRICK_SIZE * FATHOM_PHYSICAL_BRICK_SIZE) /* 1000 */

#define FATHOM_BRICK_MAP_INDEX_AIR 0         /* EMPTY (far outside): Skip Atlas Data */
#define FATHOM_BRICK_MAP_INDEX_SOLID 0xFFFF  /* SOLID (far inside): Skip Atlas Data */
#define FATHOM_BRICK_MAP_INDEX_USEFUL 0xFFFE /* USABLE: Sentinal for useable brick. Use this for Atlas Data */

typedef f32 (*fathom_sparse_grid_distance_function)(fathom_vec3 position, void *user_data);

typedef struct fathom_sparse_grid
{
    /* First Pass: Evaluate Brick Map */
    u32 brick_map_dimensions;
    u32 brick_map_bytes;
    u32 brick_map_active_bricks_count;
    u16 *brick_map_data;

    /* Second Pass: Fill Atlas */
    u32 atlas_bricks_per_row;
    u32 atlas_width;
    u32 atlas_height;
    u32 atlas_depth;
    u32 atlas_bytes;
    u8 *atlas_data;

    /* Data for shader upload */
    fathom_vec3 start;
    f32 cell_size;
    f32 truncation_distance;

    /* Internal: General data */
    fathom_vec3 center;
    u32 cell_count;
    f32 brick_radius;
    f32 cull_threshold;

} fathom_sparse_grid;

FATHOM_API u8 fathom_sparse_grid_initialize(fathom_sparse_grid *grid, fathom_vec3 grid_center, u32 grid_cell_count, f32 grid_cell_size)
{
    /* First Pass: Calculate Brick Map Memory Requirements */
    grid->brick_map_dimensions = grid_cell_count / FATHOM_BRICK_SIZE;
    grid->brick_map_bytes = grid->brick_map_dimensions * grid->brick_map_dimensions * grid->brick_map_dimensions * sizeof(u16);

    /* Data for shader upload */
    grid->start = fathom_vec3_subf(grid_center, (f32)grid_cell_count * grid_cell_size * 0.5f);
    grid->cell_size = grid_cell_size;
    grid->truncation_distance = grid_cell_size * 4.0f;

    /* Internal helpers */
    grid->center = grid_center;
    grid->cell_count = grid_cell_count;
    grid->brick_radius = (FATHOM_PHYSICAL_BRICK_SIZE * 0.5f) * 1.7320508f * grid_cell_size;
    grid->cull_threshold = grid->brick_radius + grid->truncation_distance;

    return 1;
}

FATHOM_API u8 fathom_sparse_grid_pass_01_fill_brick_map(fathom_sparse_grid *grid, fathom_sparse_grid_distance_function distance_function, void *user_data)
{
    u32 brick_map_index = 0;
    u32 active_brick_count = 0;

    u32 bx, by, bz;
    f32 px, py, pz;

    fathom_vec3 start = grid->start;

    f32 brick_step = (f32)FATHOM_BRICK_SIZE * grid->cell_size;
    f32 center_off = brick_step * 0.5f;

    pz = start.z;

    for (bz = 0; bz < grid->brick_map_dimensions; ++bz, pz += brick_step)
    {
        py = start.y;

        for (by = 0; by < grid->brick_map_dimensions; ++by, py += brick_step)
        {
            px = start.x;

            for (bx = 0; bx < grid->brick_map_dimensions; ++bx, px += brick_step, ++brick_map_index)
            {
                fathom_vec3 center = fathom_vec3_init(px + center_off, py + center_off, pz + center_off);

                f32 distance = distance_function(center, user_data);

                if (fathom_absf(distance) > grid->cull_threshold)
                {
                    /* Culled: Either air (0) or solid (0xFFFF) */
                    u16 state = (distance > 0.0f) ? FATHOM_BRICK_MAP_INDEX_AIR : FATHOM_BRICK_MAP_INDEX_SOLID;
                    grid->brick_map_data[brick_map_index] = state;
                }
                else
                {
                    /* Active: sentinel so the next pass knows to calculate it */
                    u16 state = FATHOM_BRICK_MAP_INDEX_USEFUL;
                    grid->brick_map_data[brick_map_index] = state;
                    active_brick_count++;
                }
            }
        }
    }

    grid->brick_map_active_bricks_count = active_brick_count;

    /* Calculate atlas dimensions and bytes required (e.g. how big does the atlas 3d texture needs to be to fit all relevant bricks)*/
    {
        u32 bricks_per_row = (u32)fathom_ceilf(fathom_sqrtf((f32)active_brick_count));
        u32 bricks_per_col = (active_brick_count + bricks_per_row - 1) / bricks_per_row;

        grid->atlas_bricks_per_row = bricks_per_row;
        grid->atlas_width = bricks_per_row * FATHOM_PHYSICAL_BRICK_SIZE;
        grid->atlas_height = bricks_per_col * FATHOM_PHYSICAL_BRICK_SIZE;
        grid->atlas_depth = FATHOM_PHYSICAL_BRICK_SIZE;
        grid->atlas_bytes = grid->atlas_width * grid->atlas_height * grid->atlas_depth * sizeof(u8);
    }

    return 1;
}

FATHOM_API u8 fathom_sparse_grid_pass_02_fill_atlas(fathom_sparse_grid *grid, fathom_sparse_grid_distance_function distance_function, void *user_data)
{
    u32 bricks_per_row = grid->atlas_bricks_per_row;
    u32 atlas_used_count = 0;
    f32 quant_scale = 127.5f / grid->truncation_distance;
    f32 apron_offset = -((f32)FATHOM_BRICK_APRON * grid->cell_size);
    u32 atlas_width = bricks_per_row * FATHOM_PHYSICAL_BRICK_SIZE;
    u32 atlas_height = ((grid->brick_map_active_bricks_count + bricks_per_row - 1) / bricks_per_row) * FATHOM_PHYSICAL_BRICK_SIZE;

    u32 bx, by, bz;
    u32 lx, ly, lz;

    for (bz = 0; bz < grid->brick_map_dimensions; ++bz)
    {
        for (by = 0; by < grid->brick_map_dimensions; ++by)
        {
            for (bx = 0; bx < grid->brick_map_dimensions; ++bx)
            {
                u32 map_idx = bx + (by * grid->brick_map_dimensions) + (bz * grid->brick_map_dimensions * grid->brick_map_dimensions);

                fathom_vec3 brick_min;
                u32 cur_idx;
                u32 atlas_bx, atlas_by;
                u32 atlas_vox_stride;
                u32 atlas_slice_stride;

                if (grid->brick_map_data[map_idx] != FATHOM_BRICK_MAP_INDEX_USEFUL)
                {
                    continue;
                }

                /* 1. Determine World Space origin for this brick (including apron) */
                brick_min = fathom_vec3_init(
                    grid->start.x + (f32)(bx * FATHOM_BRICK_SIZE) * grid->cell_size,
                    grid->start.y + (f32)(by * FATHOM_BRICK_SIZE) * grid->cell_size,
                    grid->start.z + (f32)(bz * FATHOM_BRICK_SIZE) * grid->cell_size);

                /* 2. Determine Atlas Destination (Brick Coordinates) */
                cur_idx = atlas_used_count++;
                atlas_bx = cur_idx % bricks_per_row;
                atlas_by = cur_idx / bricks_per_row;

                /* 3. Voxel Fill Loop */
                atlas_vox_stride = atlas_width;
                atlas_slice_stride = atlas_width * atlas_height;

                for (lz = 0; lz < FATHOM_PHYSICAL_BRICK_SIZE; ++lz)
                {
                    f32 pz = brick_min.z + apron_offset + ((f32)lz * grid->cell_size);

                    for (ly = 0; ly < FATHOM_PHYSICAL_BRICK_SIZE; ++ly)
                    {
                        f32 py = brick_min.y + apron_offset + ((f32)ly * grid->cell_size);

                        /* Calculate destination pointer for this row (x-line) in the atlas */
                        u32 dst_x = atlas_bx * FATHOM_PHYSICAL_BRICK_SIZE;
                        u32 dst_y = (atlas_by * FATHOM_PHYSICAL_BRICK_SIZE) + ly;
                        u32 dst_z = lz;

                        u8 *dst_row = &grid->atlas_data[dst_x + (dst_y * atlas_vox_stride) + (dst_z * atlas_slice_stride)];

                        for (lx = 0; lx < FATHOM_PHYSICAL_BRICK_SIZE; ++lx)
                        {
                            f32 px = brick_min.x + apron_offset + ((f32)lx * grid->cell_size);

                            f32 dist = distance_function(fathom_vec3_init(px, py, pz), user_data);

                            /* Quantize: map [-trunc, +trunc] to [0, 255] */
                            f32 val = (dist * quant_scale) + 127.5f;

                            if (val < 0.0f)
                            {
                                val = 0.0f;
                            }
                            if (val > 255.0f)
                            {
                                val = 255.0f;
                            }

                            dst_row[lx] = (u8)val;
                        }
                    }
                }

                /* 4. Update Map with 1-based index to Atlas Brick */
                grid->brick_map_data[map_idx] = (u16)(cur_idx + 1);
            }
        }
    }

    return 1;
}

#endif /* FATHOM_SPARSE_GRID_H */