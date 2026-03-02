#ifndef FATHOM_COLOR_H
#define FATHOM_COLOR_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] Color Range Mapping
 * #############################################################################
 */
/* fathom_color_map_range(u8_value, 0.0f, 255.0f, -1.0f, 1.0f); */
FATHOM_API FATHOM_INLINE f32 fathom_color_map_range(f32 val, f32 in_min, f32 in_max, f32 out_min, f32 out_max)
{
    return out_min + ((out_max - out_min) / (in_max - in_min)) * (val - in_min);
}

/* 0–255 → -1.0–1.0 */
FATHOM_API FATHOM_INLINE f32 fathom_color_u8_to_f32_snorm(u8 value)
{
    return ((f32)value * (2.0f / 255.0f)) - 1.0f;
}

/* 0–255 → 0.0–1.0 */
FATHOM_API FATHOM_INLINE f32 fathom_color_u8_to_f32_unorm(u8 value)
{
    return (f32)value * (1.0f / 255.0f);
}

/* -1.0–1.0 → 0–255 (clamped) */
FATHOM_API FATHOM_INLINE u8 fathom_color_f32_to_u8_snorm(f32 value)
{
    f32 normalized = (value + 1.0f) * 0.5f;

    normalized = (normalized < 0.0f) ? 0.0f : (normalized > 1.0f) ? 1.0f
                                                                  : normalized;

    return (u8)(normalized * 255.0f + 0.5f);
}

/* 0.0–1.0 → 0–255 (clamped) */
FATHOM_API FATHOM_INLINE u8 fathom_color_f32_to_u8_unorm(f32 value)
{
    value = (value < 0.0f) ? 0.0f : (value > 1.0f) ? 1.0f
                                                   : value;
    return (u8)(value * 255.0f + 0.5f);
}

/* #############################################################################
 * # [SECTION] Color Packing & Conversion
 * #############################################################################
 */
/* Pack 0-255 components into a 0xRRGGBBAA integer */
FATHOM_API FATHOM_INLINE u32 fathom_color_pack_rgba(u8 r, u8 g, u8 b, u8 a)
{
    return ((u32)r << 24) | ((u32)g << 16) | ((u32)b << 8) | (u32)a;
}

/* Unpack a specific component from a 0xRRGGBBAA integer */
FATHOM_API FATHOM_INLINE u8 fathom_color_get_r(u32 rgba)
{
    return (u8)((rgba >> 24) & 0xFF);
}

FATHOM_API FATHOM_INLINE u8 fathom_color_get_g(u32 rgba)
{
    return (u8)((rgba >> 16) & 0xFF);
}

FATHOM_API FATHOM_INLINE u8 fathom_color_get_b(u32 rgba)
{
    return (u8)((rgba >> 8) & 0xFF);
}

FATHOM_API FATHOM_INLINE u8 fathom_color_get_a(u32 rgba)
{
    return (u8)(rgba & 0xFF);
}

#endif /* FATHOM_COLOR_H */