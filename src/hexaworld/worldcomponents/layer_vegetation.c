
#include "hexaworldcomponents.h"

#include <math.h>

#include <raylib.h>

#include <colorpalette.h>

#define ITERATION_NB_VEGETATION (10u)    ///< number of automaton iteration for the vegetation layer

// -------------------------------------------------------------------------------------------------
// -- VEGETATION -----------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void vegetation_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = AS_RAYLIB_COLOR(COLOR_LEAFY_GREEN);

    tile_color.a = (u8) ((f32) tile_color.a * cell->vegetation_cover);

    if (cell->altitude <= 0) {
        tile_color = AS_RAYLIB_COLOR(COLOR_CERULEAN);
    }

    DrawPoly(
            *((Vector2 *) &target_shape->center),
            HEXAGON_SIDES_NB,
            target_shape->radius,
            0.0f,
            tile_color
    );

    tile_color.a = (cell->vegetation_cover > VEGETATION_CUTOUT_THRESHOLD) * 0xFF;
    DrawPolyLines(
            *((Vector2 *) &target_shape->center),
            HEXAGON_SIDES_NB,
            5*target_shape->radius/6,
            0.0f,
            tile_color
    );
}

// -------------------------------------------------------------------------------------------------
static void vegetation_seed(hexaworld_t *world) {
    
    ratio_t humidity_rating = 0.0f;
    ratio_t temperature_rating = 0.0f;

    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            humidity_rating = (world->tiles[x][y].humidity * (world->tiles[x][y].freshwater_height == 0u)) + (world->tiles[x][y].freshwater_height > 0u);
            temperature_rating = 
                    NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, world->tiles[x][y].temperature / 2)
                    / NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, VEGETATION_TEMPERATURE_MEAN);

            world->tiles[x][y].vegetation_cover = humidity_rating * temperature_rating;
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void vegetation_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    f32 vegetation_mean = 0.0f;
    size_t nb_cells_with_vegetation = 0u;

    if (cell->vegetation_cover > VEGETATION_CUTOUT_THRESHOLD) {
        return;
    }
    
    for (size_t i = 0 ; i < DIRECTIONS_NB; i++) {
        tmp_cell = (hexa_cell_t *) neighbors[i];

        vegetation_mean += tmp_cell->vegetation_cover;
        nb_cells_with_vegetation += (tmp_cell->vegetation_cover > VEGETATION_CUTOUT_THRESHOLD);
    }
    vegetation_mean /= nb_cells_with_vegetation;

    cell->vegetation_cover = vegetation_mean 
                    * (NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, cell->temperature)
                    / NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, VEGETATION_TEMPERATURE_MEAN));
}

const layer_calls_t vegetation_layer_calls = {
        .draw_func          = &vegetation_draw,
        .seed_func          = &vegetation_seed,
        .automaton_func     = &vegetation_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_VEGETATION,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};