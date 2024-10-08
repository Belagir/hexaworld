
#include "hexaworldcomponents.h"

#include <math.h>
#include <stdlib.h>

#include <raylib.h>

#include <colorpalette.h>

#define ITERATION_NB_TELLURIC (5u)    ///< number of automaton iteration for the telluric layer

// -------------------------------------------------------------------------------------------------
// -- TELLURIC -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void telluric_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    vector_2d_cartesian_t translated_vec = { 0u };
    Color tile_color = AS_RAYLIB_COLOR(COLOR_WHITE);

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_TELLURIC_RIDGE)) {
        tile_color = AS_RAYLIB_COLOR(COLOR_TANGERINE);
    } else if (hexa_cell_has_flag(cell, HEXAW_FLAG_TELLURIC_RIFT)) {
        tile_color = AS_RAYLIB_COLOR(COLOR_AQUA_GREEN);
    }

    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(tile_color), 1.0f , DRAW_HEXAGON_FILL);

    translated_vec = vector2d_polar_to_cartesian(cell->telluric_vector);
    DrawLineV(
            *((Vector2*) (&target_shape->center)), 
            (Vector2) { 
                    .x = target_shape->center.v + translated_vec.v * target_shape->radius, 
                    .y = target_shape->center.w + translated_vec.w * target_shape->radius },
            AS_RAYLIB_COLOR(COLOR_TOMATO)
    );
}

// -------------------------------------------------------------------------------------------------
static void telluric_seed(hexaworld_t *world) {
    size_t nb_seeds = 0u;
    size_t x_random = 0u;
    size_t y_random = 0u;

    // choosing a number of seeds, with a minimum required number
    nb_seeds = (size_t) (TELLURIC_SEED_NB_PER_TILE * world->height * world->width);
    if (nb_seeds < TELLURIC_SEED_NB_MIN) {
        nb_seeds = TELLURIC_SEED_NB_MIN;
    }

    // initialising the array
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].telluric_vector = (vector_2d_polar_t) { 
                    .angle = 0.0f,
                    .magnitude = 0.0f
            };
        }
    }

    // putting about the good number of seeds
    for (size_t i = 0u ; i < nb_seeds ; i++) {
        x_random = rand() % world->width;
        y_random = rand() % world->height;

        world->tiles[x_random][y_random].telluric_vector = (vector_2d_polar_t) {
                .angle = (rand() % TELLURIC_VECTOR_DIRECTIONS_NB) * TELLURIC_VECTOR_UNIT_ANGLE,
                .magnitude = 1.0f
        };
    }
}

// -------------------------------------------------------------------------------------------------
static void telluric_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    u32 angle_counter[TELLURIC_VECTOR_DIRECTIONS_NB] = { 0u };
    hexa_cell_t *tmp_cell = NULL;
    size_t index_of_most_present = 0u;
    u32 one_neighbor_is_set = 0u;
    size_t angle_index = 0u;

    // the objective is to set the telluric vector's direction to the most represented direction around it
    // to make the seeds grow into zones. Once a cell's direction is set, it cannot change. We use the 
    // magnitude as a flag to know if a vector is set, because all telluric vectors must have a magnitude 
    // of 1.

    // if the current cell already has a direction, nothing is to be done
    if (!float_equal(cell->telluric_vector.magnitude, 0.0f, 1u)) {
        return;
    }

    // computing the most represented angle 
    for (size_t i = 0u ; i < DIRECTIONS_NB; i++) {
        // capturing the neighboring cell
        tmp_cell = (hexa_cell_t *) neighbors[i];
        // if the neighboring cell is unset, we ignore it
        if (float_equal(tmp_cell->telluric_vector.magnitude, 0.0f, 1u)) {
            continue;
        }
        
        // we need to know if at least one of the neighbor propagated its value to our cell
        one_neighbor_is_set = 1u;

        // counting the number of times the angle is encountered
        angle_index = (size_t) (tmp_cell->telluric_vector.angle / TELLURIC_VECTOR_UNIT_ANGLE);
        angle_counter[angle_index] += 1u;

        // remembering the most represented angle
        if (angle_counter[angle_index] > angle_counter[index_of_most_present]) {
            index_of_most_present = angle_index;
        }
    }
    
    if (one_neighbor_is_set) {
        cell->telluric_vector.angle = index_of_most_present * TELLURIC_VECTOR_UNIT_ANGLE;
        cell->telluric_vector.magnitude = 1.0f;
    }
}

// -------------------------------------------------------------------------------------------------
static void telluric_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    size_t pushed_against_cells[2u] = { 0u };
    f32 pushed_against_ratios[2u] = { 0u };
    size_t pushed_from_cells[2u] = { 0u };
    f32 pushed_from_ratios[2u] = { 0u };

    hexa_cell_get_surrounding_cells_pointed(cell->telluric_vector.angle, pushed_against_cells, pushed_against_ratios);
    hexa_cell_get_surrounding_cells_pointed(cell->telluric_vector.angle + PI, pushed_from_cells, pushed_from_ratios);

    for (size_t i = 0u ; i < 2u ; i++) {
        if (!float_equal(cell->telluric_vector.angle, ((hexa_cell_t *) neighbors[pushed_from_cells[i]])->telluric_vector.angle, 1u)) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_TELLURIC_RIDGE);
        } else if (!float_equal(cell->telluric_vector.angle, ((hexa_cell_t *) neighbors[pushed_against_cells[i]])->telluric_vector.angle, 1u)) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_TELLURIC_RIFT);
        }
    }
}

const layer_calls_t telluric_layer_calls = {
        .draw_func          = &telluric_draw,
        .seed_func          = &telluric_seed,
        .automaton_func     = &telluric_apply,
        .flag_gen_func      = &telluric_flag_gen, 
        .automaton_iter     = ITERATION_NB_TELLURIC,
        .iteration_flavour  = LAYER_GEN_ITERATE_RELATIVE
};