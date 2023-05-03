
#include "hexaworldcomponents.h"

#include <math.h>
#include <stdlib.h>

#include <raylib.h>

#include <colorpalette.h>

#define ITERATION_NB_WINDS (1u)       ///< number of automaton iteration for the winds layer

/**
 * @brief Cells directions needed for the automaton function
 */
typedef enum two_cells_directions_t {
    /// left cell
    LEFT_CELL,
    /// right cell
    RIGHT_CELL,
    /// number of directions
    TWO_CELLS
} two_cells_directions_t;

// -------------------------------------------------------------------------------------------------
// -- WINDS -------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void winds_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    vector_2d_cartesian_t translated_vec = { 0u };
    Color terrain_color = AS_RAYLIB_COLOR(COLOR_GRAY);
    terrain_color.a = (u8) (((f32) (cell->altitude * (cell->altitude > 0)) / (f32) ALTITUDE_MAX) * 0xFF );

    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(terrain_color), 1.0f , DRAW_HEXAGON_FILL);

    DrawCircle(target_shape->center.v, target_shape->center.w, (target_shape->radius/2)*(1.0f-cell->winds_vector.magnitude), AS_RAYLIB_COLOR(COLOR_AZURE));

    translated_vec = vector2d_polar_to_cartesian(cell->winds_vector);
    DrawLineV(
            *((Vector2*) (&target_shape->center)), 
            (Vector2) { 
                    .x = target_shape->center.v + translated_vec.v * target_shape->radius, 
                    .y = target_shape->center.w + translated_vec.w * target_shape->radius },
            AS_RAYLIB_COLOR(COLOR_DUSK_BLUE)
    );
}

// -------------------------------------------------------------------------------------------------
static void winds_seed(hexaworld_t *world) {
    // reap the storm ?
    f32 starting_angle = ((f32) (rand() % WINDS_VECTOR_DIRECTIONS_NB)) * (WINDS_VECTOR_UNIT_ANGLE);
    
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].winds_vector = (vector_2d_polar_t) {
                    .angle = starting_angle + (PI * ((f32) (world->tiles[x][y].temperature - TEMPERATURE_MIN) / (f32) TEMPERATURE_RANGE)),
                    .magnitude = 1.0f
            };
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void winds_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    f32 x_axis_sum = 0.0f;
    f32 y_axis_sum = 0.0f;
    f32 mean_weight = 0.0f;
    f32 mean_angle = 0.0f;
    f32 normalized_altitude_diff = 0.0f;

    // since the angle is not always divisible by 6, the wind goes to two cells
    size_t possible_directions[TWO_CELLS] = { 0u };
    i32 cell_altitudes[TWO_CELLS] = { 0 };
    // cell index later chosen as the least resistance
    size_t definitive_direction = 0u;

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = ((hexa_cell_t *) neighbors[i]);
        x_axis_sum += cos(tmp_cell->winds_vector.angle);
        y_axis_sum += sin(tmp_cell->winds_vector.angle);
    }

    mean_weight = 1.0f / (f32) DIRECTIONS_NB;
    mean_angle = atan2f(mean_weight * x_axis_sum, mean_weight * y_axis_sum);
    cell->winds_vector.angle = mean_angle;

    // leftmost cell
    possible_directions[LEFT_CELL] = (size_t) ceilf((cell->winds_vector.angle / (PI_T_2)) * (f32) DIRECTIONS_NB) % DIRECTIONS_NB;
    cell_altitudes[LEFT_CELL] = ((hexa_cell_t *) neighbors[possible_directions[LEFT_CELL]])->altitude;
    cell_altitudes[LEFT_CELL] *= (cell_altitudes[LEFT_CELL] > 0); /* altitude below 0 has water over it */
    // rightmost cell
    possible_directions[RIGHT_CELL] = (size_t) floorf((cell->winds_vector.angle / (PI_T_2)) * (f32) DIRECTIONS_NB) % DIRECTIONS_NB;
    cell_altitudes[RIGHT_CELL] = ((hexa_cell_t *) neighbors[possible_directions[RIGHT_CELL]])->altitude;
    cell_altitudes[RIGHT_CELL] *= (cell_altitudes[RIGHT_CELL] > 0); /* altitude below 0 has water over it */

    definitive_direction = (cell_altitudes[LEFT_CELL] < cell_altitudes[RIGHT_CELL])
            ? possible_directions[LEFT_CELL]
            : possible_directions[RIGHT_CELL];
    
    // difference between the two "winded upon" cells
    normalized_altitude_diff = (f32) abs(cell_altitudes[LEFT_CELL] - cell_altitudes[RIGHT_CELL]) / (f32) ALTITUDE_MAX;
    cell->winds_vector.angle += ((definitive_direction / (f32) DIRECTIONS_NB) * (PI_T_2)) * normalized_altitude_diff;

    // difference between the current cell's altitude and the main winded upon cell
    normalized_altitude_diff = (f32) abs(cell->altitude * (cell->altitude > 0) - cell_altitudes[definitive_direction]) / (f32) ALTITUDE_MAX;
    cell->winds_vector.magnitude = (1.0f - (normalized_altitude_diff) * (normalized_altitude_diff > 0.25f));
}

const layer_calls_t winds_layer_calls = {
        .draw_func          = &winds_draw,
        .seed_func          = &winds_seed,
        .automaton_func     = &winds_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_WINDS,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};
