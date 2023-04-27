
#include <raylib.h>
#include "hexaworldcomponents.h"

#define ITERATION_NB_ALTITUDE (10u)   ///< number of automaton iteration for the altitude layer

static void altitude_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void altitude_seed(hexaworld_t *world);

static void altitude_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -- ALTITUDE -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void altitude_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color base_color = { 0u };
    f32 color_intensity = 0.0f;

    base_color = (Color) { 0x21, 0x36, 0x8F, 0xFF, };
    color_intensity = (f32) cell->altitude / (f32) ALTITUDE_MIN;

    if (cell->altitude > 0) {
        base_color = (Color) { 0x90, 0x5F, 0x07, 0xFF, };
        color_intensity = (f32) cell->altitude / (f32) ALTITUDE_MAX;
    }
    base_color.a = 64u + (u8) ((color_intensity) * (f32) (255u-64u));

    DrawPoly(*((Vector2*) &target_shape->center), HEXAGON_SIDES_NB, target_shape->radius, 0.0f, base_color);
}

// -------------------------------------------------------------------------------------------------
static void altitude_seed(hexaworld_t *world) {
    hexa_cell_t *tmp_cell = NULL;

    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            tmp_cell = world->tiles[x] + y;

            if (hexa_cell_has_flag(tmp_cell, HEXAW_FLAG_MOUNTAIN)) {
                tmp_cell->altitude = ALTITUDE_MAX;
            } else if (hexa_cell_has_flag(tmp_cell, HEXAW_FLAG_UNDERWATER_CANYONS)) {
                tmp_cell->altitude = ALTITUDE_MIN;
            } else if (tmp_cell->altitude > 0) {
                tmp_cell->altitude = ALTITUDE_MAX / 4;
            } else {
                tmp_cell->altitude = ALTITUDE_MIN / 4;
            }
        }
    }

}

// -------------------------------------------------------------------------------------------------
static void altitude_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    i32 maximum_altitude = (i32) 0x80000000;
    i32 minimum_altitude = (i32) 0x7FFFFFFF;

    hexa_cell_t *tmp_cell = NULL;

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = ((hexa_cell_t *) neighbors[i]);

        if (tmp_cell->altitude < minimum_altitude) {
            minimum_altitude = tmp_cell->altitude;
        }
        if (tmp_cell->altitude > maximum_altitude) {
            maximum_altitude = tmp_cell->altitude;
        }
    }

    if ((cell->altitude > minimum_altitude) && (cell->altitude < maximum_altitude)) {
        cell->altitude = (maximum_altitude + minimum_altitude) / 2;
    }
}

const layer_calls_t altitude_layer_calls = {
        .draw_func          = &altitude_draw,
        .seed_func          = &altitude_seed,
        .automaton_func     = &altitude_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_ALTITUDE,
        .iteration_flavour  = LAYER_GEN_ITERATE_RELATIVE
};
