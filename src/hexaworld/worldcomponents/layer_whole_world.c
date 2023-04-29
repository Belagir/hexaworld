
#include "hexaworldcomponents.h"

#include <stdlib.h>

#include <raylib.h>
#include <colorpalette.h>

#define ITERATION_NB_WHOLE_WORLD (0u)

// -------------------------------------------------------------------------------------------------
// -- WHOLE WORLD ----------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void whole_world_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {

}

const layer_calls_t whole_world_layer_calls = {
        .draw_func          = &whole_world_draw,
        .seed_func          = NULL,
        .automaton_func     = NULL,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_WHOLE_WORLD,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};
