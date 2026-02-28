#ifndef FATHOM_TYPES_H
#define FATHOM_TYPES_H

/* #############################################################################
 * # [SECTION] Compiler Specific Settings
 * #############################################################################
 */

/* Inline */
#if __STDC_VERSION__ >= 199901L
#define FATHOM_INLINE inline
#elif defined(__GNUC__) || defined(__clang__)
#define FATHOM_INLINE __inline__
#elif defined(_MSC_VER)
#define FATHOM_INLINE __inline
#else
#define FATHOM_INLINE
#endif

/* Alignment */
#if defined(_MSC_VER)
#define FATHOM_ALIGN(x) __declspec(align(x))
#elif defined(__GNUC__) || defined(__clang__)
#define FATHOM_ALIGN(x) __attribute__((aligned(x)))
#else
#define FATHOM_ALIGN(x)
#endif

/* Architecture */
#if defined(__x86_64__) || defined(_M_X64)
#define FATHOM_ARCH_X64
#elif defined(__aarch64__) || defined(_M_ARM64)
#define FATHOM_ARCH_ARM64
#endif

/* API */
#define FATHOM_API static
#define FATHOM_NULL (void *)0

/* #############################################################################
 * # [SECTION] Basic Types
 * #############################################################################
 */
typedef char s8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short i16;
typedef int i32;
typedef float f32;
typedef double f64;

#define FATHOM_TYPES_STATIC_ASSERT(c, m) typedef char fathom_types_assert_##m[(c) ? 1 : -1]
FATHOM_TYPES_STATIC_ASSERT(sizeof(s8) == 1, s8_size_must_be_1);
FATHOM_TYPES_STATIC_ASSERT(sizeof(u8) == 1, u8_size_must_be_1);
FATHOM_TYPES_STATIC_ASSERT(sizeof(u16) == 2, u16_size_must_be_2);
FATHOM_TYPES_STATIC_ASSERT(sizeof(i16) == 2, i16_size_must_be_2);
FATHOM_TYPES_STATIC_ASSERT(sizeof(u32) == 4, u32_size_must_be_4);
FATHOM_TYPES_STATIC_ASSERT(sizeof(i32) == 4, i32_size_must_be_4);
FATHOM_TYPES_STATIC_ASSERT(sizeof(f32) == 4, f32_size_must_be_4);
FATHOM_TYPES_STATIC_ASSERT(sizeof(f64) == 8, f64_size_must_be_8);
#undef FATHOM_TYPES_STATIC_ASSERT

#endif /* FATHOM_TYPES_H */