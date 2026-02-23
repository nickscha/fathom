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

/* #############################################################################
 * # [SECTION] Basic Math (General)
 * #############################################################################
 */
#define FATHOM_PI2 6.28318530717958647692f
#define FATHOM_PI_HALF 1.57079632679489661923f
#define FATHOM_SIN_LUT_SIZE 256

static f32 fathom_sin_lut[FATHOM_SIN_LUT_SIZE] = {
    0.0000f, 0.0245f, 0.0491f, 0.0736f, 0.0980f, 0.1224f, 0.1467f, 0.1710f,
    0.1951f, 0.2191f, 0.2430f, 0.2667f, 0.2903f, 0.3137f, 0.3369f, 0.3599f,
    0.3827f, 0.4052f, 0.4276f, 0.4496f, 0.4714f, 0.4929f, 0.5141f, 0.5350f,
    0.5556f, 0.5758f, 0.5957f, 0.6152f, 0.6344f, 0.6532f, 0.6716f, 0.6895f,
    0.7071f, 0.7242f, 0.7409f, 0.7572f, 0.7730f, 0.7883f, 0.8032f, 0.8176f,
    0.8315f, 0.8449f, 0.8577f, 0.8701f, 0.8819f, 0.8932f, 0.9040f, 0.9142f,
    0.9239f, 0.9330f, 0.9415f, 0.9495f, 0.9569f, 0.9638f, 0.9700f, 0.9757f,
    0.9808f, 0.9853f, 0.9892f, 0.9925f, 0.9952f, 0.9973f, 0.9988f, 0.9997f,
    1.0000f, 0.9997f, 0.9988f, 0.9973f, 0.9952f, 0.9925f, 0.9892f, 0.9853f,
    0.9808f, 0.9757f, 0.9700f, 0.9638f, 0.9569f, 0.9495f, 0.9415f, 0.9330f,
    0.9239f, 0.9142f, 0.9040f, 0.8932f, 0.8819f, 0.8701f, 0.8577f, 0.8449f,
    0.8315f, 0.8176f, 0.8032f, 0.7883f, 0.7730f, 0.7572f, 0.7409f, 0.7242f,
    0.7071f, 0.6895f, 0.6716f, 0.6532f, 0.6344f, 0.6152f, 0.5957f, 0.5758f,
    0.5556f, 0.5350f, 0.5141f, 0.4929f, 0.4714f, 0.4496f, 0.4276f, 0.4052f,
    0.3827f, 0.3599f, 0.3369f, 0.3137f, 0.2903f, 0.2667f, 0.2430f, 0.2191f,
    0.1951f, 0.1710f, 0.1467f, 0.1224f, 0.0980f, 0.0736f, 0.0491f, 0.0245f,
    0.0000f, -0.0245f, -0.0491f, -0.0736f, -0.0980f, -0.1224f, -0.1467f, -0.1710f,
    -0.1951f, -0.2191f, -0.2430f, -0.2667f, -0.2903f, -0.3137f, -0.3369f, -0.3599f,
    -0.3827f, -0.4052f, -0.4276f, -0.4496f, -0.4714f, -0.4929f, -0.5141f, -0.5350f,
    -0.5556f, -0.5758f, -0.5957f, -0.6152f, -0.6344f, -0.6532f, -0.6716f, -0.6895f,
    -0.7071f, -0.7242f, -0.7409f, -0.7572f, -0.7730f, -0.7883f, -0.8032f, -0.8176f,
    -0.8315f, -0.8449f, -0.8577f, -0.8701f, -0.8819f, -0.8932f, -0.9040f, -0.9142f,
    -0.9239f, -0.9330f, -0.9415f, -0.9495f, -0.9569f, -0.9638f, -0.9700f, -0.9757f,
    -0.9808f, -0.9853f, -0.9892f, -0.9925f, -0.9952f, -0.9973f, -0.9988f, -0.9997f,
    -1.0000f, -0.9997f, -0.9988f, -0.9973f, -0.9952f, -0.9925f, -0.9892f, -0.9853f,
    -0.9808f, -0.9757f, -0.9700f, -0.9638f, -0.9569f, -0.9495f, -0.9415f, -0.9330f,
    -0.9239f, -0.9142f, -0.9040f, -0.8932f, -0.8819f, -0.8701f, -0.8577f, -0.8449f,
    -0.8315f, -0.8176f, -0.8032f, -0.7883f, -0.7730f, -0.7572f, -0.7409f, -0.7242f,
    -0.7071f, -0.6895f, -0.6716f, -0.6532f, -0.6344f, -0.6152f, -0.5957f, -0.5758f,
    -0.5556f, -0.5350f, -0.5141f, -0.4929f, -0.4714f, -0.4496f, -0.4276f, -0.4052f,
    -0.3827f, -0.3599f, -0.3369f, -0.3137f, -0.2903f, -0.2667f, -0.2430f, -0.2191f,
    -0.1951f, -0.1710f, -0.1467f, -0.1224f, -0.0980f, -0.0736f, -0.0491f, -0.0245f};

FATHOM_API FATHOM_INLINE f32 fathom_sinf(f32 x)
{
    f32 index, frac;
    i32 i, i2;

    x -= FATHOM_PI2 * (f32)((i32)(x * (1.0f / FATHOM_PI2)));

    if (x < 0)
    {
        x += FATHOM_PI2;
    }

    index = x * (FATHOM_SIN_LUT_SIZE / FATHOM_PI2);
    i = (i32)index;
    frac = index - (f32)i;

    i &= (FATHOM_SIN_LUT_SIZE - 1);
    i2 = (i + 1) & (FATHOM_SIN_LUT_SIZE - 1);

    return fathom_sin_lut[i] + frac * (fathom_sin_lut[i2] - fathom_sin_lut[i]);
}

FATHOM_API FATHOM_INLINE f32 fathom_cosf(f32 x)
{
    return fathom_sinf(x + FATHOM_PI_HALF);
}

FATHOM_API FATHOM_INLINE f32 fathom_sminf(f32 a, f32 b, f32 k)
{
    f32 h = fathom_maxf(k - fathom_absf(a - b), 0.0f) / k;
    return fathom_minf(a, b) - h * h * h * k * (1.0f / 6.0f);
}

#endif /* FATHOM_MATH_BASIC_SCALAR_H */