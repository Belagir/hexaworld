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
hexagon_shape_t hexagon_pixel_position_in_rectangle(f32 boundaries[4u], u32 x, u32 y, u32 width, u32 height) {
    hexagon_shape_t shape = { 0u };
    vector_2d_cartesian_t centering_offset = { 0u };

    shape.radius = (width >= height)
            ? (boundaries[2u] / (((f32) width + 0.5f) * SQRT_OF_3))
            : (boundaries[3u] / ((f32) height * THREE_HALVES));

    centering_offset.v = (boundaries[2u] - (((f32) width + 0.5f) * (shape.radius * SQRT_OF_3)))    * 0.5f;
    centering_offset.w = (boundaries[3u] - (((f32) height)       * (shape.radius * THREE_HALVES))) * 0.5f;

    shape.center = (vector_2d_cartesian_t) { 
            .v = boundaries[0u] + ((((f32) x + 0.5f) + (0.5f * (f32) (y & 0x01))) * SQRT_OF_3 * shape.radius) + centering_offset.v, 
            .w = boundaries[1u] + (((f32) y + 0.5f) * THREE_HALVES * shape.radius) + centering_offset.w
    };

    return shape;
}

// -------------------------------------------------------------------------------------------------
vector_2d_cartesian_t hexagon_array_coords_from_rectangle(f32 boundaries[4u], u32 pix_x, u32 pix_y, u32 array_width, u32 array_height) {
    vector_2d_cartesian_t coords = { 0u };
    f32 x;
    f32 y;
    f32 cell_radius = 0.0f;
    vector_2d_cartesian_t centering_offset = { 0u };

    cell_radius = (array_width >= array_height)
            ? (boundaries[2u] / (((f32) array_width + 0.5f) * SQRT_OF_3))
            : (boundaries[3u] / ((f32) array_height * THREE_HALVES));

    centering_offset.v = (boundaries[2u] - (((f32) array_width + 0.5f) * (cell_radius * SQRT_OF_3)))    * 0.5f;
    centering_offset.w = (boundaries[3u] - (((f32) array_height)       * (cell_radius * THREE_HALVES))) * 0.5f;

    y = ((f32) pix_y - centering_offset.w - boundaries[1u]) / (THREE_HALVES * cell_radius);
    x = (((f32) pix_x - centering_offset.v - boundaries[0u]) / (SQRT_OF_3    * cell_radius)) - (0.5f * (f32) ((u32) y & 0x01));

    coords = (vector_2d_cartesian_t) { .v = x, .w = y };

    return coords;
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
