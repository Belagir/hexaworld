
#include "hexaworldcomponents.h"

#define SQRT_OF_3 (1.73205f)      ///< approximation of the square root of 3
#define THREE_HALVES (1.5f)       ///< not an *approximation* of 3 / 2

// -------------------------------------------------------------------------------------------------
void hexa_cell_set_flag(hexa_cell_t *cell, u32 flag) {
    cell->flags = (cell->flags | (0x01 << flag));
}

// -------------------------------------------------------------------------------------------------
u32 hexa_cell_has_flag(hexa_cell_t *cell, u32 flag) {
    return (cell->flags & (0x01 << flag));
}

// -------------------------------------------------------------------------------------------------
hexagon_shape_t hexagon_position_in_rectangle(f32 boundaries[4u], u32 x, u32 y, u32 width, u32 height) {
    hexagon_shape_t shape = { 0u };

    shape.radius = (boundaries[3u] / ((f32) height*THREE_HALVES));

    shape.center = (vector_2d_cartesian_t) { 
            boundaries[0u] + ((((f32) x+0.5f) + (0.5f * (f32) (y & 0x01))) * SQRT_OF_3 * (shape.radius)), 
            boundaries[1u] + ((((f32) y+0.5f)) * THREE_HALVES * (shape.radius)) 
    };

    return shape;
}
