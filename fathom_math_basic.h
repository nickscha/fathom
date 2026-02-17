#ifndef FATHOM_MATH_BASIC_H
#define FATHOM_MATH_BASIC_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] Basic Math
 * #############################################################################
 */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4699) /* MSVC-specific aliasing warning */
#endif
FATHOM_API f32 fathom_invsqrtf(f32 number)
{
    union
    {
        f32 f;
        i32 i;
    } conv;

    f32 x2, y;
    const f32 threehalfs = 1.5F;

    x2 = number * 0.5F;
    conv.f = number;
    conv.i = 0x5f3759df - (conv.i >> 1); /* Magic number for approximation */
    y = conv.f;
    y = y * (threehalfs - (x2 * y * y)); /* One iteration of Newton's method */

    return (y);
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

FATHOM_API f32 fathom_sqrtf(f32 x)
{
    return (x * fathom_invsqrtf(x));
}

FATHOM_API f32 fathom_cbrtf(f32 x)
{
    f32 y;
    i32 i;

    if (x == 0.0f)
    {
        return 0.0f;
    }

    y = x;

    for (i = 0; i < 8; ++i)
    {
        y = (2.0f * y + x / (y * y)) / 3.0f;
    }

    return y;
}

#endif /* FATHOM_MATH_BASIC_H */