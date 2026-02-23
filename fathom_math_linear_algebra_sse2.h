#ifndef FATHOM_MATH_LINEAR_ALGEBRA_SSE2_H
#define FATHOM_MATH_LINEAR_ALGEBRA_SSE2_H

#include "fathom_math_basic.h"

/* #############################################################################
 * # [SECTION] Linear Algebra Math (SSE2)
 * #############################################################################
 */
#include <emmintrin.h>

typedef struct FATHOM_ALIGN(16) fathom_vec3
{
    f32 x;
    f32 y;
    f32 z;
    f32 w; /* padding */

} fathom_vec3;

static fathom_vec3 fathom_vec3_zero = {0.0f, 0.0f, 0.0f, 0.0f};

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_init(f32 x, f32 y, f32 z)
{
    fathom_vec3 result;

    __m128 res;

    res = _mm_set_ps(0.0f, z, y, x);
    _mm_store_ps((f32 *)&result, res);

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_sub(fathom_vec3 a, fathom_vec3 b)
{
    fathom_vec3 result;

    __m128 va, vb, res;

    va = _mm_load_ps((const f32 *)&a);
    vb = _mm_load_ps((const f32 *)&b);
    res = _mm_sub_ps(va, vb);

    _mm_store_ps((f32 *)&result, res);
    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_subf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    __m128 va, vval, res;

    va = _mm_load_ps((const f32 *)&a);
    vval = _mm_set_ps(0.0f, value, value, value);
    res = _mm_sub_ps(va, vval);

    _mm_store_ps((f32 *)&result, res);
    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_addf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    __m128 va, vval, res;

    va = _mm_load_ps((const f32 *)&a);
    vval = _mm_set_ps(0.0f, value, value, value);
    res = _mm_add_ps(va, vval);

    _mm_store_ps((f32 *)&result, res);
    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_abs(fathom_vec3 a)
{
    fathom_vec3 result;

    __m128 va, mask, res;

    va = _mm_load_ps((const f32 *)&a);
    mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)); /* Clear sign bit */
    res = _mm_and_ps(va, mask);

    _mm_store_ps((f32 *)&result, res);
    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_maxf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    __m128 va, vval, res;

    va = _mm_load_ps((const f32 *)&a);
    vval = _mm_set_ps(0.0f, value, value, value);
    res = _mm_max_ps(va, vval);

    _mm_store_ps((f32 *)&result, res);
    return result;
}

FATHOM_API FATHOM_INLINE f32 fathom_vec3_dot(fathom_vec3 a, fathom_vec3 b)
{
    __m128 va, vb, mul, mask, shuf1, sum1, shuf2, sum2;

    va = _mm_load_ps((const f32 *)&a);
    vb = _mm_load_ps((const f32 *)&b);
    mul = _mm_mul_ps(va, vb);

    mask = _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1));
    mul = _mm_and_ps(mul, mask);

    shuf1 = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 3, 0, 1));
    sum1 = _mm_add_ps(mul, shuf1);
    shuf2 = _mm_shuffle_ps(sum1, sum1, _MM_SHUFFLE(1, 0, 3, 2));
    sum2 = _mm_add_ps(sum1, shuf2);

    return _mm_cvtss_f32(sum2);
}

FATHOM_API FATHOM_INLINE f32 fathom_vec3_length(fathom_vec3 a)
{
    __m128 va, mul, mask, shuf1, sum1, shuf2, sum2, root;

    va = _mm_load_ps((const f32 *)&a);
    mul = _mm_mul_ps(va, va);

    mask = _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1));
    mul = _mm_and_ps(mul, mask);

    shuf1 = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 3, 0, 1));
    sum1 = _mm_add_ps(mul, shuf1);
    shuf2 = _mm_shuffle_ps(sum1, sum1, _MM_SHUFFLE(1, 0, 3, 2));
    sum2 = _mm_add_ps(sum1, shuf2);

    root = _mm_sqrt_ss(sum2);
    return _mm_cvtss_f32(root);
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_cross(fathom_vec3 a, fathom_vec3 b)
{
    fathom_vec3 result;

    __m128 va, vb, a_yzx, b_zxy, a_zxy, b_yzx, mul1, mul2, sub;

    va = _mm_load_ps((const f32 *)&a);
    vb = _mm_load_ps((const f32 *)&b);

    a_yzx = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 0, 2, 1));
    b_zxy = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 1, 0, 2));
    mul1 = _mm_mul_ps(a_yzx, b_zxy);

    a_zxy = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 1, 0, 2));
    b_yzx = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 0, 2, 1));
    mul2 = _mm_mul_ps(a_zxy, b_yzx);

    sub = _mm_sub_ps(mul1, mul2);

    _mm_store_ps((f32 *)&result, sub);
    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_normalize(fathom_vec3 a)
{
    fathom_vec3 result;

    __m128 va, mul, mask, shuf1, sum1, shuf2, length_sq;
    __m128 rsqrt, half, three_halfs, half_length_sq, rsqrt_sq, nr, zero, cmp, res;

    va = _mm_load_ps((const f32 *)&a);
    mul = _mm_mul_ps(va, va);

    mask = _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1));
    mul = _mm_and_ps(mul, mask);

    shuf1 = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 3, 0, 1));
    sum1 = _mm_add_ps(mul, shuf1);
    shuf2 = _mm_shuffle_ps(sum1, sum1, _MM_SHUFFLE(1, 0, 3, 2));
    length_sq = _mm_add_ps(sum1, shuf2);

    rsqrt = _mm_rsqrt_ps(length_sq);

    half = _mm_set1_ps(0.5f);
    three_halfs = _mm_set1_ps(1.5f);
    half_length_sq = _mm_mul_ps(length_sq, half);
    rsqrt_sq = _mm_mul_ps(rsqrt, rsqrt);
    nr = _mm_sub_ps(three_halfs, _mm_mul_ps(half_length_sq, rsqrt_sq));
    rsqrt = _mm_mul_ps(rsqrt, nr);

    zero = _mm_setzero_ps();
    cmp = _mm_cmpgt_ps(length_sq, zero);
    rsqrt = _mm_and_ps(rsqrt, cmp);

    res = _mm_mul_ps(va, rsqrt);
    _mm_store_ps((f32 *)&result, res);

    return result;
}

#endif /* FATHOM_MATH_LINEAR_ALGEBRA_SSE2_H */