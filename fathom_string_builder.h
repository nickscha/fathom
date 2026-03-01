#ifndef FATHOM_STRING_BUILDER_H
#define FATHOM_STRING_BUILDER_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] String Builder
 * #############################################################################
 */
typedef enum fathom_sb_pad
{
    FATHOM_SB_PAD_LEFT,
    FATHOM_SB_PAD_RIGHT
} fathom_sb_pad;

typedef struct fathom_sb
{
    u32 size;
    u32 length;
    s8 *buffer;

} fathom_sb;

FATHOM_API void fathom_sb_s8(fathom_sb *sb, s8 *s)
{
    u32 len = sb->length;
    u32 cap = sb->size;

    if (len >= cap)
    {
        return;
    }

    while (*s && (len + 1 < cap))
    {
        sb->buffer[len++] = *s++;
    }

    sb->buffer[len] = 0;
    sb->length = len;
}

FATHOM_API void fathom_sb_s8_pad(fathom_sb *sb, s8 *s, u32 total_width, s8 pad_char, fathom_sb_pad side)
{
    u32 s_len = 0;
    s8 *temp = s;
    u32 pad_count;
    u32 i;

    while (temp[s_len])
    {
        s_len++;
    }

    pad_count = (total_width > s_len) ? (total_width - s_len) : 0;

    if (sb->length + s_len + pad_count >= sb->size)
    {
        return;
    }

    if (side == FATHOM_SB_PAD_LEFT)
    {
        for (i = 0; i < pad_count; ++i)
        {
            sb->buffer[sb->length++] = pad_char;
        }
        while (*s)
        {
            sb->buffer[sb->length++] = *s++;
        }
    }
    else
    {
        while (*s)
        {
            sb->buffer[sb->length++] = *s++;
        }
        for (i = 0; i < pad_count; ++i)
        {
            sb->buffer[sb->length++] = pad_char;
        }
    }

    /* Null terminate and update length */
    sb->buffer[sb->length] = 0;
}

FATHOM_API void fathom_sb_i32(fathom_sb *sb, i32 v)
{
    s8 tmp[12];
    i32 i = 0;
    u32 u;
    u32 len = sb->length;
    u32 cap = sb->size;

    if (len + 1 >= cap)
    {
        return;
    }

    if (v < 0)
    {
        sb->buffer[len++] = '-';
        u = (u32)(-v);
    }
    else
    {
        u = (u32)v;
    }

    if (u == 0)
    {
        sb->buffer[len++] = '0';
        sb->buffer[len] = 0;
        sb->length = len;
        return;
    }

    while (u && i < 12)
    {
        tmp[i++] = (s8)('0' + (u % 10));
        u /= 10;
    }

    while (i-- && (len + 1 < cap))
    {
        sb->buffer[len++] = tmp[i];
    }

    sb->buffer[len] = 0;
    sb->length = len;
}

FATHOM_API void fathom_sb_f64(fathom_sb *sb, f64 v, i32 decimals)
{
    i32 i;
    f64 frac;
    u32 len = sb->length;
    u32 cap = sb->size;

    if (len + 1 >= cap)
    {
        return;
    }

    if (v < 0.0)
    {
        sb->buffer[len++] = '-';
        v = -v;
    }

    sb->length = len;
    fathom_sb_i32(sb, (i32)v);
    len = sb->length;

    if (len + 1 >= cap)
    {
        return;
    }

    sb->buffer[len++] = '.';

    frac = v - (f64)((i32)v);

    for (i = 0; i < decimals && (len + 1 < cap); ++i)
    {
        frac *= 10.0;
        sb->buffer[len++] = (s8)('0' + (i32)frac);
        frac -= (i32)frac;
    }

    sb->buffer[len] = 0;
    sb->length = len;
}

#endif /* FATHOM_STRING_BUILDER_H */