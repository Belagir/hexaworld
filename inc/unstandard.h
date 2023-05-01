/**
 * @file unstandard.h
 * @author gabriel 
 * @brief Quick and dirty redefinitions for ease of code without always including some std lib header.
 * @version 0.1
 * @date 2023-04-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __UNSTANDARD_H__
#define __UNSTANDARD_H__

#ifndef NULL
#define NULL 0x0 ///< sneaky redifinition of NULL so I won't have to import stdlib just for it
#endif

#define SQRT_OF_3 (1.73205f)      ///< approximation of the square root of 3
#define THREE_HALVES (1.5f)       ///< not an *approximation* of 3 / 2

/// returns the sign bitfield of a 32-bit signed integer
#define SGN_I32(_v) ((_v) & (0x80000000))

/// Calculates a normal distribution of a certain mean and variance (not squared variance !). Needs math.h.
#define NORMAL_DISTRIBUTION(_mean, _variance, _x) ((1.0f / (_variance * sqrt(2.0f * (PI)))) * exp(-0.5f * pow(((_x) - (_mean)) / (_variance), 2)))

typedef char  i8;   ///< convenience type redefinition
typedef short i16;  ///< convenience type redefinition
typedef int   i32;  ///< convenience type redefinition
typedef long  i64;  ///< convenience type redefinition

typedef unsigned char  u8;  ///< convenience type redefinition
typedef unsigned short u16; ///< convenience type redefinition
typedef unsigned int   u32; ///< convenience type redefinition
typedef unsigned long  u64; ///< convenience type redefinition

typedef u64 size_t; ///< unsigned integer of maximum size

typedef float  f32; ///< convenience type redefinition
typedef double f64; ///< convenience type redefinition

/**
 * @brief Vector as a two cartesian coordinates.
 * Colinear with the raylib Vector2 type.
 */
typedef struct vector_2d_cartesian_t {
    f32 v;
    f32 w;
} vector_2d_cartesian_t;

/**
 * @brief Vector as a two polar coordinates.
 */
typedef struct vector_2d_polar_t {
    f32 angle;
    f32 magnitude;
} vector_2d_polar_t;

/**
 * @brief Copies byte to byte from `source` to `dest`.
 * The size of `source` and `dest` must both be of at least `nb_bytes`, and `source` must be after `dest` if the two regions overlap.
 * 
 * @param[out] dest pointer to the start of the copied-on region
 * @param[in] source pointer to the start of the copied-from region
 * @param[in] nb_bytes number of bytes copied.
 */
void bytewise_copy(void *dest, void *source, size_t nb_bytes);

/**
 * @brief Converts a 2d vector from a polar coordinate pair to a cartesian one.
 * 
 * @param[in] vec vector to convert
 * @return vector_2d_cartesian_t polar coordinates equivalent to the supplied vector
 */
vector_2d_cartesian_t vector2d_polar_to_cartesian(vector_2d_polar_t vec);

/**
 * @brief Converts a 2d vector from a cartesian coordinate pair to a polar one.
 * 
 * @param[in] vec vector to convert
 * @return vector_2d_polar_t cartesian coordinates equivalent to the supplied vector
 */
vector_2d_polar_t vector2d_cartesian_to_polar(vector_2d_cartesian_t vec);

/**
 * @brief Checks for equality between two floats.
 * 
 * @param[in] val1 first float
 * @param[in] val2 second float
 * @param[in] max_ulps_diff maximum difference in bit representation
 * @return u32 0 if unequal, 1 if equal (up to the max difference)
 */
u32 float_equal(f32 val1, f32 val2, u32 max_ulps_diff);

/**
 * @brief Returns the minimum of two numbers.
 * 
 * @param[in] v1 first number
 * @param[in] v2 second number
 * @return u64 the smallest of the two numbers.
 */
u64 min(u64 v1, u64 v2);

#endif