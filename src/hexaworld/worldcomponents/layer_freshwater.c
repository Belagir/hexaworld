
#include "hexaworldcomponents.h"

#define ITERATION_NB_FRESHWATER (20u)    ///< number of automaton iteration for the freshwater layer

static void freshwater_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void freshwater_seed(hexaworld_t *world);

static void freshwater_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

static void freshwater_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {

}

static void freshwater_seed(hexaworld_t *world){

}

static void freshwater_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]){

}


const layer_calls_t freshwater_layer_calls = {
        .draw_func          = &freshwater_draw,
        .seed_func          = &freshwater_seed,
        .automaton_func     = &freshwater_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_FRESHWATER,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};