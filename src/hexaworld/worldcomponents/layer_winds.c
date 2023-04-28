
#include "hexaworldcomponents.h"

#include <math.h>
#include <stdlib.h>

#include <raylib.h>

#define ITERATION_NB_WINDS (1u)       ///< number of automaton iteration for the winds layer

static void winds_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void winds_seed(hexaworld_t *world);

static void winds_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);


// -------------------------------------------------------------------------------------------------
// -- WINDS -------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void winds_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    vector_2d_cartesian_t translated_vec = { 0u };

    DrawPoly(*(
            (Vector2*) &target_shape->center), 
            HEXAGON_SIDES_NB, 
            target_shape->radius, 
            0.0f, 
            (Color) { 200u, 200u, 200u, (u8) (((f32) (cell->altitude * (cell->altitude > 0)) / (f32) ALTITUDE_MAX) * 255u )}
    );

    DrawCircle(target_shape->center.v, target_shape->center.w, (target_shape->radius/2)*(1.0f-cell->winds_vector.magnitude), (Color) { 0x6e, 0xBA, 0xFF, 0xFF });

    translated_vec = vector2d_polar_to_cartesian(cell->winds_vector);
    DrawLineV(
            *((Vector2*) (&target_shape->center)), 
            (Vector2) { 
                    .x = target_shape->center.v + translated_vec.v * target_shape->radius, 
                    .y = target_shape->center.w + translated_vec.w * target_shape->radius },
            DARKBLUE
    );
}

// -------------------------------------------------------------------------------------------------
static void winds_seed(hexaworld_t *world) {
    // reap the storm ?
    f32 starting_angle = ((f32) (rand() % WINDS_VECTOR_DIRECTIONS_NB)) * (WINDS_VECTOR_UNIT_ANGLE);
    const i32 temperature_range = TEMPERATURE_MAX - TEMPERATURE_MIN;
    
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].winds_vector = (vector_2d_polar_t) {
                    .angle = starting_angle + (PI * ((f32) (world->tiles[x][y].temperature - TEMPERATURE_MIN) / (f32) temperature_range)),
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
    size_t possible_directions[2u] = { 0u };
    i32 cell_altitudes[2u] = { 0 };
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
    possible_directions[0u] = (size_t) ceilf((cell->winds_vector.angle / (PI_T_2)) * (f32) DIRECTIONS_NB) % DIRECTIONS_NB;
    cell_altitudes[0u] = ((hexa_cell_t *) neighbors[possible_directions[0u]])->altitude;
    cell_altitudes[0u] *= (cell_altitudes[0u] > 0); /* altitude below 0 has water over it */
    // rightmost cell
    possible_directions[1u] = (size_t) floorf((cell->winds_vector.angle / (PI_T_2)) * (f32) DIRECTIONS_NB) % DIRECTIONS_NB;
    cell_altitudes[1u] = ((hexa_cell_t *) neighbors[possible_directions[1u]])->altitude;
    cell_altitudes[1u] *= (cell_altitudes[1u] > 0); /* altitude below 0 has water over it */

    definitive_direction = (cell_altitudes[0u] < cell_altitudes[1u])
            ? possible_directions[0u]
            : possible_directions[1u];
    
    // difference between the two "winded upon" cells
    normalized_altitude_diff = (f32) abs(cell_altitudes[0u] - cell_altitudes[1u]) / (f32) ALTITUDE_MAX;
    cell->winds_vector.angle += ((definitive_direction / (f32) DIRECTIONS_NB) * (PI_T_2)) * normalized_altitude_diff;

    // difference between the current cell's altitude and the main winded upon cell
    normalized_altitude_diff = (f32) abs(cell->altitude * (cell->altitude > 0) - cell_altitudes[definitive_direction]) / (f32) ALTITUDE_MAX;
    cell->winds_vector.magnitude = (1.0f - normalized_altitude_diff);
}

const layer_calls_t winds_layer_calls = {
        .draw_func          = &winds_draw,
        .seed_func          = &winds_seed,
        .automaton_func     = &winds_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_WINDS,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};
