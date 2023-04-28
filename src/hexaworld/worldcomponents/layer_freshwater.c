
#include "hexaworldcomponents.h"

#include <stdlib.h>
#include <math.h>

#include <raylib.h>

#define ITERATION_NB_FRESHWATER (50u)    ///< number of automaton iteration for the freshwater layer

static void freshwater_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void freshwater_seed(hexaworld_t *world);

static void freshwater_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------

static i32 cell_total_height(hexa_cell_t *cell);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void freshwater_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {

    vector_2d_cartesian_t translated_vec = { 0u };

    char buffer_water_height[24u] = { 0u };

    // landmass 
    if (cell->altitude > 0) {
        DrawPoly(
                *((Vector2 *) &target_shape->center),
                HEXAGON_SIDES_NB,
                target_shape->radius,
                0.0f,
                (Color) { .r = 0xAB, .g = 0xAB, .b = 0xAB, .a = 0xFF }
        );
    }

    if (cell->freshwater_height > 0u) {
        translated_vec = vector2d_polar_to_cartesian((vector_2d_polar_t) { 
                .angle = ((f32) cell->freshwater_direction / (f32) DIRECTIONS_NB) * PI_T_2,
                .magnitude = 1.0f
        } );
        DrawLineEx(
                *((Vector2 *) &target_shape->center),
                (Vector2) { 
                        .x = target_shape->center.v + translated_vec.v * target_shape->radius, 
                        .y = target_shape->center.w + translated_vec.w * target_shape->radius },
                5.0f,
                (Color) { .r = 0x00, .g = 0xD9, .b = 0xF9, .a = 0xFF }
        );

        DrawText(TextFormat("%dm", cell->freshwater_height), target_shape->center.v, target_shape->center.w, (i32) target_shape->radius/2, BLACK);
    }
}

// -------------------------------------------------------------------------------------------------
static void freshwater_seed(hexaworld_t *world){
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {

            if (world->tiles[x][y].altitude <= 0) {
                continue;
            }

            if (world->tiles[x][y].precipitations > FRESHWATER_PRECIPITATIONS_THRESHOLD) {
                world->tiles[x][y].freshwater_height = FRESHWATER_SOURCE_START_DEPTH;
            } else if (hexa_cell_has_flag(world->tiles[x] + y, HEXAW_FLAG_MOUNTAIN)) {
                world->tiles[x][y].freshwater_height = ((rand() % FRESHWATER_MOUNTAIN_NO_SOURCE_CHANCE) == 0) * FRESHWATER_SOURCE_START_DEPTH;
            }

            world->tiles[x][y].freshwater_direction = rand() % DIRECTIONS_NB;
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void freshwater_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]){
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    hexa_cell_t *flowed_to_cell = NULL;

    size_t lowest_alt_direction = 0u;
    i32 lowest_alt = 0;

    size_t reversed_direction = 0u;

    // this is a ocean tile !
    if (cell->altitude <= 0) {
        return;
    }

    if (cell->freshwater_height > 0u) {
        flowed_to_cell = (hexa_cell_t *) neighbors[cell->freshwater_direction];
        
        if (cell_total_height(flowed_to_cell) >= cell_total_height(cell)) {

            lowest_alt = 0x7FFFFFFF;
            for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
                tmp_cell = (hexa_cell_t *) neighbors[i];
                if (cell_total_height(tmp_cell) < lowest_alt) {
                    lowest_alt = cell_total_height(tmp_cell);
                    lowest_alt_direction = i;
                }
            }

            cell->freshwater_direction = lowest_alt_direction;
            flowed_to_cell = (hexa_cell_t *) neighbors[lowest_alt_direction];

            if (cell_total_height(flowed_to_cell) >= cell_total_height(cell)) {
                cell->freshwater_height = cell_total_height(flowed_to_cell) - cell->altitude + 1u;
            }
        }
    } else {
        for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
            tmp_cell = (hexa_cell_t *) neighbors[i];

            if (tmp_cell->freshwater_height == 0u) {
                continue;
            }

            reversed_direction = (i + (DIRECTIONS_NB/2)) % DIRECTIONS_NB;

            if ((tmp_cell->freshwater_direction == reversed_direction) && (cell_total_height(tmp_cell) > cell_total_height(cell))) {
                cell->freshwater_height = FRESHWATER_SOURCE_START_DEPTH;
            }
        }

    }
}

// -------------------------------------------------------------------------------------------------
static i32 cell_total_height(hexa_cell_t *cell) {
    return (cell->altitude + (i32) cell->freshwater_height);
}


const layer_calls_t freshwater_layer_calls = {
        .draw_func          = &freshwater_draw,
        .seed_func          = &freshwater_seed,
        .automaton_func     = &freshwater_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_FRESHWATER,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};