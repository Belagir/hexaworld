
#include "hexaworldcomponents.h"

#include <raylib.h>

#define ITERATION_NB_VEGETATION (0u)    ///< number of automaton iteration for the vegetation layer

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
}

// -------------------------------------------------------------------------------------------------
static void vegetation_seed(hexaworld_t *world) {
    
    f32 humidity_rating = 0.0f;
    f32 temperature_rating = 0.0f;

    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            humidity_rating = world->tiles[x][y].humidity;
            // temperature_rating = (world->tiles[x][y].temperature);
        }
    }
}

const layer_calls_t vegetation_layer_calls = {
        .draw_func          = &vegetation_draw,
        .seed_func          = &vegetation_seed,
        .automaton_func     = NULL,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_VEGETATION,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};