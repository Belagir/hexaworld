/**
 * @file hexagonparadigm.h
 * @author gabriel 
 * @brief The hexagon is the start and the end. This definition file contains some functions and data types needed to work with hexagons.
 * @version 0.1
 * @date 2023-04-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __HEXAGONPARADIGM_H__
#define __HEXAGONPARADIGM_H__

#include <unstandard.h>

#define HEXAGON_SIDES_NB (6u)     ///< number of sides of an hexagon. tough.

typedef enum hexagon_draw_fillmode_t {
    DRAW_HEXAGON_FILL,
    DRAW_HEXAGON_LINES
} hexagon_draw_fillmode_t;

/**
 * @brief just the shape of an hexagon.
 */
typedef struct hexagon_shape_t {
    vector_2d_cartesian_t center;
    f32 radius;
} hexagon_shape_t;

/**
 * @brief Computes the position and radius of a hexagonal cell in a pixel rectangle.
 * 
 * @param[in] boundaries rectangle defined by the topleft coordinates and its sides' length (in pixels)
 * @param[in] x tile x-position in the world array
 * @param[in] y tile y-position in the world array
 * @param[in] width width, in tiles, of the world array
 * @param[in] height height, in tiles, of the world array
 * @return hexagon_shape_t pixel shape of the hexagon representing the cell's coordinates
 */
hexagon_shape_t hexagon_position_in_rectangle(f32 boundaries[4u], u32 x, u32 y, u32 width, u32 height);

/**
 * @brief 
 * 
 * @param target_shape 
 * @param color 
 * @param scale 
 * @param fill 
 */
void draw_hexagon(hexagon_shape_t *target_shape, u32 color, f32 scale, hexagon_draw_fillmode_t fill);

#endif
