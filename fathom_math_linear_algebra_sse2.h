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

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_initf(f32 value)
{
    fathom_vec3 result;

    __m128 res;

    res = _mm_set_ps(0.0f, value, value, value);
    _mm_store_ps((f32 *)&result, res);

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_add(fathom_vec3 a, fathom_vec3 b)
{
    fathom_vec3 result;

    __m128 va, vb, res;

    va = _mm_load_ps((const f32 *)&a);
    vb = _mm_load_ps((const f32 *)&b);
    res = _mm_add_ps(va, vb);

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

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_mul(fathom_vec3 a, fathom_vec3 b)
{
    fathom_vec3 result;

    __m128 va, vb, res;

    va = _mm_load_ps((const f32 *)&a);
    vb = _mm_load_ps((const f32 *)&b);
    res = _mm_mul_ps(va, vb);

    _mm_store_ps((f32 *)&result, res);

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_div(fathom_vec3 a, fathom_vec3 b)
{
    fathom_vec3 result;

    __m128 va, vb, res;

    va = _mm_load_ps((const f32 *)&a);
    vb = _mm_load_ps((const f32 *)&b);
    res = _mm_div_ps(va, vb);

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

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_mulf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    __m128 va;
    __m128 vval;
    __m128 res;

    va = _mm_load_ps((f32 *)&a);
    vval = _mm_set1_ps(value);
    res = _mm_mul_ps(va, vval);

    _mm_store_ps((f32 *)&result, res);

    return result;
}

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_divf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;
    __m128 va, vval, res;

    va = _mm_load_ps((f32 *)&a);
    vval = _mm_set1_ps(value); /* Broadcast value to all 4 slots */
    res = _mm_div_ps(va, vval);

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

FATHOM_API FATHOM_INLINE fathom_vec3 fathom_vec3_minf(fathom_vec3 a, f32 value)
{
    fathom_vec3 result;

    __m128 va, vval, res;

    va = _mm_load_ps((const f32 *)&a);
    vval = _mm_set_ps(0.0f, value, value, value);
    res = _mm_min_ps(va, vval);

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

#define FATHOM_MAT2X2_ELEMENT_COUNT 4

#ifdef FATHOM_MAT_ROW_MAJOR_ORDER
#define FATHOM_MAT2X2_AT(row, col) ((row) * 2 + (col)) /* Row-major order */
#else
#define FATHOM_MAT2X2_AT(row, col) ((col) * 2 + (row)) /* Column-major order */
#endif

typedef struct FATHOM_ALIGN(16) fathom_mat2x2
{
    f32 e[FATHOM_MAT2X2_ELEMENT_COUNT];

} fathom_mat2x2;

FATHOM_API FATHOM_INLINE fathom_mat2x2 fathom_mat2x2_init(f32 m00, f32 m01, f32 m10, f32 m11)
{
    fathom_mat2x2 result;

    __m128 m;

#ifdef FATHOM_MAT_ROW_MAJOR_ORDER
    /* Row-major: [m11, m10, m01, m00] */
    m = _mm_set_ps(m11, m10, m01, m00);
#else
    /* Column-major: [m11, m01, m10, m00] */
    m = _mm_set_ps(m11, m01, m10, m00);
#endif

    _mm_store_ps(result.e, m);

    return result;
}

FATHOM_API FATHOM_INLINE fathom_mat2x2 fathom_mat2x2_rot2d(f32 angle)
{
    f32 s = fathom_sinf(angle);
    f32 c = fathom_cosf(angle);

    return fathom_mat2x2_init(c, -s, s, c);
}

FATHOM_API FATHOM_INLINE void fathom_vec2_mul_mat2x2(f32 *a, f32 *b, fathom_mat2x2 m)
{
    __m128 r0, r1, v_x, v_y, res;

    r0 = _mm_set_ps(0.0f, 0.0f, m.e[FATHOM_MAT2X2_AT(0, 1)], m.e[FATHOM_MAT2X2_AT(0, 0)]);
    r1 = _mm_set_ps(0.0f, 0.0f, m.e[FATHOM_MAT2X2_AT(1, 1)], m.e[FATHOM_MAT2X2_AT(1, 0)]);

    v_x = _mm_set1_ps(*a);
    v_y = _mm_set1_ps(*b);

    res = _mm_add_ps(_mm_mul_ps(v_x, r0), _mm_mul_ps(v_y, r1));

    *a = _mm_cvtss_f32(res);
    *b = _mm_cvtss_f32(_mm_shuffle_ps(res, res, _MM_SHUFFLE(1, 1, 1, 1)));
}

#define FATHOM_MAT4X4_ELEMENT_COUNT 16

#ifdef FATHOM_MAT_ROW_MAJOR_ORDER
#define FATHOM_MAT4X4_AT(row, col) ((row) * 4 + (col)) /* Row-major order */
#else
#define FATHOM_MAT4X4_AT(row, col) ((col) * 4 + (row)) /* Column-major order */
#endif

typedef struct fathom_mat4x4
{
    f32 e[FATHOM_MAT4X4_ELEMENT_COUNT];
} fathom_mat4x4;

static fathom_mat4x4 fathom_mat4x4_zero =
    {{0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f}};

FATHOM_API FATHOM_INLINE fathom_mat4x4 fathom_mat4x4_mul(fathom_mat4x4 a, fathom_mat4x4 b)
{
    fathom_mat4x4 r;

    i32 i;

    __m128 a0, a1, a2, a3;
    __m128 b_row_col, b0, b1, b2, b3, res;

#ifdef FATHOM_MAT_ROW_MAJOR_ORDER
    b0 = _mm_loadu_ps(&b.e[0]);
    b1 = _mm_loadu_ps(&b.e[4]);
    b2 = _mm_loadu_ps(&b.e[8]);
    b3 = _mm_loadu_ps(&b.e[12]);

    for (i = 0; i < 4; ++i)
    {
        b_row_col = _mm_loadu_ps(&a.e[i * 4]);

        a0 = _mm_shuffle_ps(b_row_col, b_row_col, _MM_SHUFFLE(0, 0, 0, 0));
        a1 = _mm_shuffle_ps(b_row_col, b_row_col, _MM_SHUFFLE(1, 1, 1, 1));
        a2 = _mm_shuffle_ps(b_row_col, b_row_col, _MM_SHUFFLE(2, 2, 2, 2));
        a3 = _mm_shuffle_ps(b_row_col, b_row_col, _MM_SHUFFLE(3, 3, 3, 3));

        res = _mm_add_ps(_mm_mul_ps(a0, b0), _mm_mul_ps(a1, b1));
        res = _mm_add_ps(res, _mm_mul_ps(a2, b2));
        res = _mm_add_ps(res, _mm_mul_ps(a3, b3));

        _mm_storeu_ps(&r.e[i * 4], res);
    }
#else
    a0 = _mm_loadu_ps(&a.e[0]);
    a1 = _mm_loadu_ps(&a.e[4]);
    a2 = _mm_loadu_ps(&a.e[8]);
    a3 = _mm_loadu_ps(&a.e[12]);

    for (i = 0; i < 4; ++i)
    {
        b_row_col = _mm_loadu_ps(&b.e[i * 4]);

        b0 = _mm_shuffle_ps(b_row_col, b_row_col, _MM_SHUFFLE(0, 0, 0, 0));
        b1 = _mm_shuffle_ps(b_row_col, b_row_col, _MM_SHUFFLE(1, 1, 1, 1));
        b2 = _mm_shuffle_ps(b_row_col, b_row_col, _MM_SHUFFLE(2, 2, 2, 2));
        b3 = _mm_shuffle_ps(b_row_col, b_row_col, _MM_SHUFFLE(3, 3, 3, 3));

        res = _mm_add_ps(_mm_mul_ps(a0, b0), _mm_mul_ps(a1, b1));
        res = _mm_add_ps(res, _mm_mul_ps(a2, b2));
        res = _mm_add_ps(res, _mm_mul_ps(a3, b3));

        _mm_storeu_ps(&r.e[i * 4], res);
    }
#endif

    return r;
}

FATHOM_API FATHOM_INLINE fathom_mat4x4 fathom_mat4x4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    fathom_mat4x4 result;
    __m128 r0, r1, r2, r3;
    __m128 v_left_top_near, v_right_bottom_far;
    __m128 v_diff, v_sum, v_inv_diff;

    v_left_top_near = _mm_set_ps(0.0f, near, bottom, left);
    v_right_bottom_far = _mm_set_ps(0.0f, far, top, right);
    v_diff = _mm_sub_ps(v_right_bottom_far, v_left_top_near);
    v_sum = _mm_add_ps(v_right_bottom_far, v_left_top_near);
    v_inv_diff = _mm_div_ps(_mm_set1_ps(1.0f), v_diff);
    result = fathom_mat4x4_zero;

#ifdef FATHOM_MAT_ROW_MAJOR_ORDER
    result.e[0] = 2.0f * _mm_cvtss_f32(v_inv_diff);
    result.e[3] = -_mm_cvtss_f32(v_sum) * _mm_cvtss_f32(v_inv_diff);
    result.e[5] = 2.0f * _mm_cvtss_f32(_mm_shuffle_ps(v_inv_diff, v_inv_diff, 1));
    result.e[7] = -_mm_cvtss_f32(_mm_shuffle_ps(v_sum, v_sum, 1)) * _mm_cvtss_f32(_mm_shuffle_ps(v_inv_diff, v_inv_diff, 1));
    result.e[10] = -2.0f * _mm_cvtss_f32(_mm_shuffle_ps(v_inv_diff, v_inv_diff, 2));
    result.e[11] = -_mm_cvtss_f32(_mm_shuffle_ps(v_sum, v_sum, 2)) * _mm_cvtss_f32(_mm_shuffle_ps(v_inv_diff, v_inv_diff, 2));
    result.e[15] = 1.0f;
#else
    r0 = _mm_set_ps(0.0f, 0.0f, 0.0f, 2.0f * _mm_cvtss_f32(v_inv_diff));
    r1 = _mm_set_ps(0.0f, 0.0f, 2.0f * _mm_cvtss_f32(_mm_shuffle_ps(v_inv_diff, v_inv_diff, 1)), 0.0f);
    r2 = _mm_set_ps(0.0f, -2.0f * _mm_cvtss_f32(_mm_shuffle_ps(v_inv_diff, v_inv_diff, 2)), 0.0f, 0.0f);
    r3 = _mm_mul_ps(_mm_set_ps(0.0f, -1.0f, -1.0f, -1.0f), _mm_mul_ps(v_sum, v_inv_diff));
    _mm_storeu_ps(&result.e[0], r0);
    _mm_storeu_ps(&result.e[4], r1);
    _mm_storeu_ps(&result.e[8], r2);
    _mm_storeu_ps(&result.e[12], r3);
    result.e[15] = 1.0f;
#endif

    return result;
}

#endif /* FATHOM_MATH_LINEAR_ALGEBRA_SSE2_H */