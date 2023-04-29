
#include "hexaworldcomponents.h"

#include <stdlib.h>

#include <raylib.h>
#include <colorpalette.h>

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

    // base color is for an ocean tile
    base_color = AS_RAYLIB_COLOR(COLOR_DUSK_BLUE);
    color_intensity = (f32) cell->altitude / (f32) ALTITUDE_MIN;

    // switch color if the tile is above ground
    if (cell->altitude > 0) {
        base_color = AS_RAYLIB_COLOR(COLOR_LEATHER);
        color_intensity = (f32) cell->altitude / (f32) ALTITUDE_MAX;
    }
    base_color.a = 0x40 + (u8) ((color_intensity) * (f32) (0xFF-0x40));

    DrawPoly(*((Vector2*) &target_shape->center), HEXAGON_SIDES_NB, target_shape->radius, 0.0f, base_color);
}

// -------------------------------------------------------------------------------------------------
static void altitude_seed(hexaworld_t *world) {
    hexa_cell_t *tmp_cell = NULL;

    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            tmp_cell = world->tiles[x] + y;

            if (hexa_cell_has_flag(tmp_cell, HEXAW_FLAG_MOUNTAIN)) {
                tmp_cell->altitude = (alt_m_t) ALTITUDE_MAX;
            } else if (hexa_cell_has_flag(tmp_cell, HEXAW_FLAG_UNDERWATER_CANYONS)) {
                tmp_cell->altitude = (alt_m_t) ALTITUDE_MIN;
            } else if (tmp_cell->altitude > 0) {
                tmp_cell->altitude = (alt_m_t) ((ALTITUDE_MAX / 4) - (rand() % ALTITUDE_EROSION_RAND_VARIATION));
            } else {
                tmp_cell->altitude = (alt_m_t) ((ALTITUDE_MIN / 4) + (rand() % ALTITUDE_EROSION_RAND_VARIATION));
            }
        }
    }

}

// -------------------------------------------------------------------------------------------------
static void altitude_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    i32 mean_altitude = 0;

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = (hexa_cell_t *) neighbors[i];

        mean_altitude += (i32) tmp_cell->altitude;
    }
    mean_altitude += (i32) cell->altitude * ALTITUDE_EROSION_INERTIA_WEIGHT;

    mean_altitude /= (DIRECTIONS_NB + ALTITUDE_EROSION_INERTIA_WEIGHT);

    if (SGN_I32((cell->altitude - 1)) != SGN_I32(mean_altitude)) {
        return;
    }

    cell->altitude = (alt_m_t) mean_altitude;
}

const layer_calls_t altitude_layer_calls = {
        .draw_func          = &altitude_draw,
        .seed_func          = &altitude_seed,
        .automaton_func     = &altitude_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_ALTITUDE,
        .iteration_flavour  = LAYER_GEN_ITERATE_RELATIVE
};
