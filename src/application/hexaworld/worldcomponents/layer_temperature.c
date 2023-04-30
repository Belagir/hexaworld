
#include "hexaworldcomponents.h"

#include <stdlib.h>

#include <raylib.h>

#include <colorpalette.h>

#define ITERATION_NB_TEMPERATURE (1u)    ///< number of automaton iteration for the landmass layer

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
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].temperature = 
                    // inverse distance to equator 
                    (1.0f - (((f32) abs((i32) y - (i32) (world->height/2))) / (f32) (world->height/2)))
                    // temperature ratio and shifting toward the colder temps
                    * TEMPERATURE_RANGE + TEMPERATURE_MIN;
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void temperature_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    if (cell->altitude <= 0) {
        return;
    }
    cell->temperature = cell->temperature + TEMPERATURE_ALTITUDE_MULTIPLIER * (cell->altitude);
}

const layer_calls_t temperature_layer_calls = {
        .draw_func = &temperature_draw,
        .seed_func = &temperature_seed,
        .automaton_func = &temperature_apply,
        .flag_gen_func = NULL,
        .automaton_iter = ITERATION_NB_TEMPERATURE,
        .iteration_flavour = LAYER_GEN_ITERATE_ABSOLUTE
};