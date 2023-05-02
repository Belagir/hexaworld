
#include "hexaworldcomponents.h"

#include <math.h>
#include <stdlib.h>

#include <raylib.h>

#include <colorpalette.h>

#define ITERATION_NB_TEMPERATURE (0u)    ///< number of automaton iteration for the landmass layer

// -------------------------------------------------------------------------------------------------
// -- TEMPERATURE ----------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void temperature_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color cold_color = AS_RAYLIB_COLOR(COLOR_AZURE);
    Color hot_color  = AS_RAYLIB_COLOR(COLOR_DARKISH_RED);

    Color tile_color = { 0u };
    u32 abs_max_temp = 0u;
    u32 abs_temp = 0u;

    if (cell->temperature > 0) {
        tile_color = hot_color;
        abs_max_temp = TEMPERATURE_MAX;
        abs_temp = cell->temperature;
    } else {
        tile_color = cold_color;
        abs_max_temp = (u32) (-(TEMPERATURE_MIN + (TEMPERATURE_ALTITUDE_MULTIPLIER*ALTITUDE_MAX)));
        abs_temp = (u32) (-cell->temperature);
    }

    tile_color.a = (u8) (((f32) abs_temp / (f32) abs_max_temp) * 0xFF);

    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(tile_color), 1.0f , DRAW_HEXAGON_FILL);    
}

// -------------------------------------------------------------------------------------------------
static void temperature_seed(hexaworld_t *world) {
    const f32 equator = (f32) world->height * 0.5f;
    const f32 temp_variance = (f32) world->height * 0.25f;

    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].temperature =
                    ((NORMAL_DISTRIBUTION(equator, temp_variance, y)
                    / NORMAL_DISTRIBUTION(equator, temp_variance, equator))
                    * TEMPERATURE_RANGE)
                    + TEMPERATURE_MIN;
            
            if ( world->tiles[x][y].altitude >= 0) {
                world->tiles[x][y].temperature += TEMPERATURE_ALTITUDE_MULTIPLIER * (world->tiles[x][y].altitude);
            }
        }
    }
}

const layer_calls_t temperature_layer_calls = {
        .draw_func = &temperature_draw,
        .seed_func = &temperature_seed,
        .automaton_func = NULL,
        .flag_gen_func = NULL,
        .automaton_iter = ITERATION_NB_TEMPERATURE,
        .iteration_flavour = LAYER_GEN_ITERATE_ABSOLUTE
};