#ifndef FATHOM_MATH_BASIC_SSE2_H
#define FATHOM_MATH_BASIC_SSE2_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] Basic Math (SSE2)
 * #############################################################################
 */
#include <emmintrin.h>

FATHOM_API FATHOM_INLINE f32 fathom_invsqrtf(f32 number)
{
    /* Use the hardware reciprocal square root instruction */
    __m128 v = _mm_set_ss(number);
    __m128 rsqrt = _mm_rsqrt_ss(v);

    /* Newton-Raphson iteration for higher precision */
    __m128 half = _mm_set_ss(0.5f);
    __m128 three_halfs = _mm_set_ss(1.5f);
    __m128 muls = _mm_mul_ss(_mm_mul_ss(v, half), _mm_mul_ss(rsqrt, rsqrt));
    rsqrt = _mm_mul_ss(rsqrt, _mm_sub_ss(three_halfs, muls));

    return _mm_cvtss_f32(rsqrt);
}

FATHOM_API FATHOM_INLINE f32 fathom_sqrtf(f32 x)
{
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
}

FATHOM_API FATHOM_INLINE f32 fathom_absf(f32 x)
{
    __m128 v = _mm_set_ss(x);
    __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
    return _mm_cvtss_f32(_mm_and_ps(v, mask));
}

FATHOM_API FATHOM_INLINE f32 fathom_minf(f32 a, f32 b)
{
    __m128 va = _mm_set_ss(a);
    __m128 vb = _mm_set_ss(b);
    return _mm_cvtss_f32(_mm_min_ss(va, vb));
}

FATHOM_API FATHOM_INLINE f32 fathom_maxf(f32 a, f32 b)
{
    __m128 va = _mm_set_ss(a);
    __m128 vb = _mm_set_ss(b);
    return _mm_cvtss_f32(_mm_max_ss(va, vb));
}

FATHOM_API FATHOM_INLINE f32 fathom_clampf(f32 x, f32 a, f32 b)
{
    __m128 vx = _mm_set_ss(x);
    __m128 va = _mm_set_ss(a);
    __m128 vb = _mm_set_ss(b);
    return _mm_cvtss_f32(_mm_min_ss(_mm_max_ss(vx, va), vb));
}

FATHOM_API FATHOM_INLINE f32 fathom_ceilf(f32 x)
{
    __m128 v = _mm_set_ss(x);
    __m128i i = _mm_cvttps_epi32(v);
    __m128 truncated = _mm_cvtepi32_ps(i);
    __m128 mask = _mm_cmplt_ss(truncated, v);
    __m128 one = _mm_set_ss(1.0f);
    __m128 adjustment = _mm_and_ps(mask, one);

    return _mm_cvtss_f32(_mm_add_ss(truncated, adjustment));
}

FATHOM_API FATHOM_INLINE f32 fathom_fmodf(f32 x, f32 y)
{
    __m128 vx, vy, v_quot, v_trunc, v_res;
    __m128i i_trunc;

    vx = _mm_set_ss(x);
    vy = _mm_set_ss(y);

    v_quot = _mm_div_ss(vx, vy);
    i_trunc = _mm_cvttps_epi32(v_quot);
    v_trunc = _mm_cvtepi32_ps(i_trunc);
    v_res = _mm_sub_ss(vx, _mm_mul_ss(v_trunc, vy));

    return _mm_cvtss_f32(v_res);
}

#endif /* FATHOM_MATH_BASIC_SSE2_H */