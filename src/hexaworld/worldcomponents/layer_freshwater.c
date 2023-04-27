
#include "hexaworldcomponents.h"

#include <stdlib.h>
#include <math.h>

#include <raylib.h>

#define ITERATION_NB_FRESHWATER (30u)    ///< number of automaton iteration for the freshwater layer

static void freshwater_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void freshwater_seed(hexaworld_t *world);

static void freshwater_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void freshwater_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {

    vector_2d_cartesian_t translated_vec = { 0u };

    // landmass 
    if (cell->altitude > 0) {
        DrawPoly(
                *((Vector2 *) &target_shape->center),
                HEXAGON_SIDES_NB,
                target_shape->radius,
                0.0f,
                (Color) { .r = 0xAB, .g = 0xAB, .b = 0xAB, .a = 0xFF }
        );
    }

    if (float_equal(cell->freshwater.magnitude, 0.0f, 1u)) {
        return;
    }

    // fullness
    DrawPoly(
            *((Vector2 *) &target_shape->center),
            HEXAGON_SIDES_NB,
            (target_shape->radius / 2.0f) * (cell->freshwater.magnitude > 1.0f),
            0.0f,
            (Color) { .r = 0x00, .g = 0xC9, .b = 0xD9, .a = 0xFF }
    );

    // angle
    translated_vec = vector2d_polar_to_cartesian((vector_2d_polar_t) { .angle = cell->freshwater.angle, .magnitude = 1.0f });
    DrawLineEx(
            *((Vector2*) (&target_shape->center)), 
            (Vector2) { 
                    .x = target_shape->center.v + translated_vec.v * target_shape->radius, 
                    .y = target_shape->center.w + translated_vec.w * target_shape->radius },
            5.0f,
            (Color) { .r = 0x00, .g = 0xD9, .b = 0xF9, .a = 0xFF }
    );
}

// -------------------------------------------------------------------------------------------------
static void freshwater_seed(hexaworld_t *world){
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {

            if (world->tiles[x][y].altitude <= 0) {
                continue;
            }

            world->tiles[x][y].freshwater.magnitude = (world->tiles[x][y].precipitations > FRESHWATER_PRECIPITATIONS_THRESHOLD) * FRESHWATER_SOURCE_START_COEF;

            if (float_equal(world->tiles[x][y].freshwater.magnitude, 0.0f, 1u) && hexa_cell_has_flag(world->tiles[x] + y, HEXAW_FLAG_MOUNTAIN)) {
                world->tiles[x][y].freshwater.magnitude = (f32) ((rand() % FRESHWATER_MOUNTAIN_NO_SOURCE_CHANCE) == 0) * FRESHWATER_SOURCE_START_COEF;
            }

            world->tiles[x][y].freshwater.angle = ((f32) (rand() % DIRECTIONS_NB) / (f32) DIRECTIONS_NB) * PI_T_2;
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void freshwater_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]){
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    size_t reverse_direction = 0u;
    size_t tmp_cell_river_direction = 0u;
    size_t lowest_altitude = 0u;

    if ((!float_equal(cell->freshwater.magnitude, 0.0f, 1u)) || (cell->altitude <= 0)) {
        return;
    }

    // searching for water nearby
    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = (hexa_cell_t *) (neighbors[i]);

        // no water on this tile
        if (float_equal(tmp_cell->freshwater.magnitude, 0.0f, 1u)) {
            continue;
        }

        tmp_cell_river_direction = (size_t) floorf((tmp_cell->freshwater.angle / PI_T_2) * DIRECTIONS_NB);
        reverse_direction = (i + (size_t) (DIRECTIONS_NB/2)) % (size_t) DIRECTIONS_NB;

        if (tmp_cell_river_direction == reverse_direction) {
            cell->freshwater.magnitude = FRESHWATER_SOURCE_START_COEF;
            cell->freshwater.angle = 0.0f;
        }
    }

    if (float_equal(cell->freshwater.magnitude, 0.0f, 1u)) {
        return;
    }

    // computing the next direction for the water to flow
    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = (hexa_cell_t *) (neighbors[i]);

        if (tmp_cell->altitude <= ((hexa_cell_t *) (neighbors[lowest_altitude]))->altitude) {
            lowest_altitude = i;
        }
    }

    cell->freshwater.angle = ((f32) lowest_altitude / (f32) DIRECTIONS_NB) * PI_T_2;

}


const layer_calls_t freshwater_layer_calls = {
        .draw_func          = &freshwater_draw,
        .seed_func          = &freshwater_seed,
        .automaton_func     = &freshwater_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_FRESHWATER,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};