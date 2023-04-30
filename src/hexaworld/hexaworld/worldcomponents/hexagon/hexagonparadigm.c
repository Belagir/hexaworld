/**
 * @file hexagonparadigm.c
 * @author gabriel 
 * @brief Definition file for hexagon-related functions
 * @version 0.1
 * @date 2023-04-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "hexagonparadigm.h"

#include <raylib.h>
#include <colorpalette.h>

// -------------------------------------------------------------------------------------------------
hexagon_shape_t hexagon_position_in_rectangle(f32 boundaries[4u], u32 x, u32 y, u32 width, u32 height) {
    hexagon_shape_t shape = { 0u };

    shape.radius = (width >= height)
            ? (boundaries[2u] / (((f32) width + 0.5f) * SQRT_OF_3))
            : (boundaries[3u] / ((f32) height * THREE_HALVES));

    shape.center = (vector_2d_cartesian_t) { 
            boundaries[0u] + ((((f32) x + 0.5f) + (0.5f * (f32) (y & 0x01))) * SQRT_OF_3 * (shape.radius)), 
            boundaries[1u] + ((((f32) y + 0.5f)) * THREE_HALVES * (shape.radius)) 
    };

    return shape;
}

// -------------------------------------------------------------------------------------------------
void draw_hexagon(hexagon_shape_t *target_shape, u32 color, f32 scale, hexagon_draw_fillmode_t fill) {
    void (*draw_func)(Vector2, i32, f32, f32, Color) = NULL;

    switch (fill) {
    case DRAW_HEXAGON_LINES: draw_func = &DrawPolyLines; break;
    default: draw_func = &DrawPoly; break;
    }

    draw_func(
            *((Vector2 *) &target_shape->center),
            HEXAGON_SIDES_NB,
            target_shape->radius * scale,
            0.0f,
            AS_RAYLIB_COLOR(color)
    );
}
