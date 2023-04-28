
#include "hexaworldcomponents.h"

#include <raylib.h>

#define ITERATION_NB_VEGETATION (6u)    ///< number of automaton iteration for the vegetation layer

static void vegetation_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void vegetation_seed(hexaworld_t *world);

static void vegetation_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void vegetation_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = (Color) { 0x03, 0xB9, 0x04, 0xFF };

    tile_color.a = (u8) ((f32) tile_color.a * cell->vegetation_cover);

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
    
    // maximum cover (densest jungle) : altitude < 1500m ; humidity > 0.3 ; precipitations > 0 ; freshwater (meanders)
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void vegetation_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {

}

const layer_calls_t vegetation_layer_calls = {
        .draw_func          = &vegetation_draw,
        .seed_func          = &vegetation_seed,
        .automaton_func     = &vegetation_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_VEGETATION,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};