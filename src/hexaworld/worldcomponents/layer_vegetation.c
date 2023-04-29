
#include "hexaworldcomponents.h"

#include <math.h>

#include <raylib.h>

#define ITERATION_NB_VEGETATION (10u)    ///< number of automaton iteration for the vegetation layer

static void vegetation_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void vegetation_seed(hexaworld_t *world);

static void vegetation_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void vegetation_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = (Color) { 0x03, 0xB9, 0x04, 0xFF };

    tile_color.a = (u8) ((f32) tile_color.a * cell->vegetation_cover);

    if (cell->altitude <= 0) {
        tile_color = (Color) { 0x21, 0x36, 0x8F, 0xFF, };
    }

    DrawPoly(
            *((Vector2 *) &target_shape->center),
            HEXAGON_SIDES_NB,
            target_shape->radius,
            0.0f,
            tile_color
    );

    tile_color.a = (cell->vegetation_cover > 0.01) * 0xFF;
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
            humidity_rating = world->tiles[x][y].humidity;
            temperature_rating = 
                    NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, world->tiles[x][y].temperature / 2)
                    / NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, VEGETATION_TEMPERATURE_MEAN);

            world->tiles[x][y].vegetation_cover = humidity_rating * temperature_rating;
            if (world->tiles[x][y].vegetation_cover < 0.01f) {
                world->tiles[x][y].vegetation_cover = 0.0f;
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void vegetation_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    size_t neigh_index = 0u;
    ratio_t vegetation_max = 0.0f;

    if ((cell->freshwater_height == 0u) || (cell->vegetation_cover > 0.1f)) {
        return;
    }
    
    for (size_t i = 0 ; i < DIRECTIONS_NB; i++) {
        tmp_cell = (hexa_cell_t *) neighbors[i];

        if (tmp_cell->vegetation_cover > vegetation_max) {
            vegetation_max = tmp_cell->vegetation_cover;
        }
    }

    cell->vegetation_cover = vegetation_max
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