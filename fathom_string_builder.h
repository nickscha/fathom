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

FATHOM_API void fathom_sb_write_raw(fathom_sb *sb, s8 *s, u32 total_width, s8 pad_char, fathom_sb_pad side)
{
    u32 s_len = 0;
    u32 pads;

    while (s[s_len])
    {
        s_len++;
    }

    pads = (total_width > s_len) ? (total_width - s_len) : 0;

    if (sb->length + s_len + pads >= sb->size)
    {
        return;
    }

    if (side == FATHOM_SB_PAD_LEFT)
    {
        while (pads--)
        {
            sb->buffer[sb->length++] = pad_char;
        }
    }

    while (*s)
    {
        sb->buffer[sb->length++] = *s++;
    }

    if (side == FATHOM_SB_PAD_RIGHT)
    {
        while (pads--)
        {
            sb->buffer[sb->length++] = pad_char;
        }
    }

    sb->buffer[sb->length] = 0;
}

FATHOM_API void fathom_sb_s8(fathom_sb *sb, s8 *s)
{
    fathom_sb_write_raw(sb, s, 0, 0, FATHOM_SB_PAD_RIGHT);
}

FATHOM_API void fathom_sb_s8_pad(fathom_sb *sb, s8 *s, u32 width, s8 p, fathom_sb_pad side)
{
    fathom_sb_write_raw(sb, s, width, p, side);
}

FATHOM_API void fathom_sb_i32_pad(fathom_sb *sb, i32 v, u32 width, s8 p, fathom_sb_pad side)
{
    s8 buf[12], *ptr = buf + 11;
    u32 u = (v < 0) ? (u32)-v : (u32)v;

    *ptr = 0;

    do
    {
        *--ptr = (s8)('0' + (u % 10));
        u /= 10;
    } while (u);

    if (v < 0)
    {
        *--ptr = '-';
    }

    fathom_sb_write_raw(sb, ptr, width, p, side);
}

FATHOM_API void fathom_sb_i32(fathom_sb *sb, i32 v)
{
    fathom_sb_i32_pad(sb, v, 0, 0, 0);
}

FATHOM_API void fathom_sb_f64_pad(fathom_sb *sb, f64 v, i32 dec, u32 width, s8 p, fathom_sb_pad side)
{
    s8 buf[64];

    fathom_sb tmp;
    tmp.size = 64;
    tmp.length = 0;
    tmp.buffer = buf;

    if (v < 0)
    {
        buf[tmp.length++] = '-';
        v = -v;
    }

    fathom_sb_i32_pad(&tmp, (i32)v, 0, 0, 0);

    if (dec > 0)
    {
        f64 frac;
        i32 i;

        buf[tmp.length++] = '.';
        frac = v - (f64)((i32)v);

        for (i = 0; i < dec; ++i)
        {
            frac *= 10.0;
            buf[tmp.length++] = (s8)('0' + (i32)frac);
            frac -= (i32)frac;
        }
    }
    buf[tmp.length] = 0;
    fathom_sb_write_raw(sb, buf, width, p, side);
}

FATHOM_API void fathom_sb_f64(fathom_sb *sb, f64 v, i32 dec)
{
    fathom_sb_f64_pad(sb, v, dec, 0, 0, 0);
}

#endif /* FATHOM_STRING_BUILDER_H */