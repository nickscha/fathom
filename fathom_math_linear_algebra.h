#ifndef FATHOM_MATH_LINEAR_ALGEBRA_H
#define FATHOM_MATH_LINEAR_ALGEBRA_H

#include "fathom_math_basic.h"

/* #############################################################################
 * # [SECTION] Linear Algebra Math (SIMD Detection)
 * #############################################################################
 */
#ifdef FATHOM_DISABLE_SIMD
#include "fathom_math_linear_algebra_scalar.h"
#elif defined(FATHOM_ARCH_X64)
#include "fathom_math_linear_algebra_sse2.h"
#endif

#endif /* FATHOM_MATH_LINEAR_ALGEBRA_H */