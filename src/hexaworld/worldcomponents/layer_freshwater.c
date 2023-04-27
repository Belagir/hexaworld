
#include "hexaworldcomponents.h"

#include <stdlib.h>

#include <raylib.h>

#define ITERATION_NB_FRESHWATER (1u)    ///< number of automaton iteration for the freshwater layer

static void freshwater_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void freshwater_seed(hexaworld_t *world);

static void freshwater_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

static void freshwater_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {

    if (cell->altitude > 0) {
        DrawPoly(
                *((Vector2 *) &target_shape->center),
                HEXAGON_SIDES_NB,
                target_shape->radius,
                0.0f,
                (Color) { .r = 0xAB, .g = 0xAB, .b = 0xAB, .a = 0xFF }
        );
    }

    if (cell->freshwater_height == 0) {
        return;
    }

    DrawPoly(
            *((Vector2 *) &target_shape->center),
            HEXAGON_SIDES_NB,
            target_shape->radius / 2.0f,
            0.0f,
            (Color) { .r = 0x00, .g = 0xC9, .b = 0xD9, .a = 0xFF }
    );
}

static void freshwater_seed(hexaworld_t *world){
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].freshwater_height =  world->tiles[x][y].precipitations * FRESHWATER_PRECIPITATIONS_MULTIPLIER;

            if ((world->tiles[x][y].freshwater_height == 0) && hexa_cell_has_flag(world->tiles[x] + y, HEXAW_FLAG_MOUNTAIN)) {
                world->tiles[x][y].freshwater_height = ((rand() % FRESHWATER_MOUNTAIN_NO_SOURCE_CHANCE) == 0);
            }
        }
    }
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