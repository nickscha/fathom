#ifndef FATHOM_MATH_BASIC_SCALAR_H
#define FATHOM_MATH_BASIC_SCALAR_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] Basic Math (Scalar)
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
FATHOM_API FATHOM_INLINE f32 fathom_invsqrtf(f32 number)
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

FATHOM_API FATHOM_INLINE f32 fathom_sqrtf(f32 x)
{
    return x * fathom_invsqrtf(x);
}

FATHOM_API FATHOM_INLINE f32 fathom_absf(f32 x)
{
    union
    {
        f32 f;
        u32 i;
    } conv;

    conv.f = x;
    conv.i &= 0x7FFFFFFF; /* Clear the sign bit */

    return conv.f;
}

FATHOM_API FATHOM_INLINE f32 fathom_minf(f32 a, f32 b)
{
    return (a < b) ? a : b;
}

FATHOM_API FATHOM_INLINE f32 fathom_maxf(f32 a, f32 b)
{
    return (a > b) ? a : b;
}

FATHOM_API FATHOM_INLINE f32 fathom_clampf(f32 x, f32 a, f32 b)
{
    f32 x1 = (x < a) ? a : x;
    return (x1 > b) ? b : x1;
}

FATHOM_API FATHOM_INLINE f32 fathom_ceilf(f32 x)
{
    i32 i = (i32)x;
    return (x > (f32)i) ? (f32)(i + 1) : (f32)i;
}

FATHOM_API FATHOM_INLINE f32 fathom_fmodf(f32 x, f32 y)
{
    f32 quotient;
    i32 i_quotient;
    f32 result;

    if (y == 0.0f)
    {
        return 0.0f;
    }

    /* 1. Calculate how many times y goes into x */
    quotient = x / y;

    /* 2. Truncate to integer */
    i_quotient = (i32)quotient;

    /* 3. Subtract the integer part of y from x */
    result = x - ((f32)i_quotient * y);

    return result;
}

#endif /* FATHOM_MATH_BASIC_SCALAR_H */