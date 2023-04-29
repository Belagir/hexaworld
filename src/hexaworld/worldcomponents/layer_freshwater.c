
#include "hexaworldcomponents.h"

#include <stdlib.h>
#include <math.h>

#include <raylib.h>

#include <colorpalette.h>

#define ITERATION_NB_FRESHWATER (50u)    ///< number of automaton iteration for the freshwater layer


// -------------------------------------------------------------------------------------------------
// -- FRESHWATER -----------------------------------------------------------------------------------

/**
 * @brief Returns the total hieght of a tile, adding the altitude to the water height.
 * 
 * @param cell 
 * @return i32 
 */
static i32 cell_total_height(hexa_cell_t *cell);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void freshwater_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {

    vector_2d_cartesian_t translated_vec = { 0u };

    Color color_line = AS_RAYLIB_COLOR(COLOR_AZURE);

    // landmass 
    if (cell->altitude > 0) {
        DrawPoly(
                *((Vector2 *) &target_shape->center),
                HEXAGON_SIDES_NB,
                target_shape->radius,
                0.0f,
                AS_RAYLIB_COLOR(COLOR_GRAY)
        );
    }

    if (cell->freshwater_height == 0u) {
        return;
    }

    translated_vec = vector2d_polar_to_cartesian((vector_2d_polar_t) { 
            .angle = ((f32) cell->freshwater_direction / (f32) DIRECTIONS_NB) * PI_T_2,
            .magnitude = 1.0f
    } );
    

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_LAKE)) {
        DrawPoly(
            *((Vector2 *) &target_shape->center),
            HEXAGON_SIDES_NB,
            target_shape->radius / 2,
            0.0f,
            AS_RAYLIB_COLOR(COLOR_AZURE)
    );
    }
    if (hexa_cell_has_flag(cell, HEXAW_FLAG_MEANDERS)) {
        color_line = AS_RAYLIB_COLOR(COLOR_AQUA_GREEN);
    }
    if (hexa_cell_has_flag(cell, HEXAW_FLAG_RIVER_MOUTH)) {
        color_line = AS_RAYLIB_COLOR(COLOR_DUSK_BLUE);
    }
    if (hexa_cell_has_flag(cell, HEXAW_FLAG_WATERFALLS)) {
        color_line = AS_RAYLIB_COLOR(COLOR_WHITE);
    }

    DrawLineEx(
            *((Vector2 *) &target_shape->center),
            (Vector2) { 
                    .x = target_shape->center.v + translated_vec.v * target_shape->radius, 
                    .y = target_shape->center.w + translated_vec.w * target_shape->radius },
            5.0f,
            color_line
    );
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
static void freshwater_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    hexa_cell_t *flowed_to_cell = NULL;

    size_t lowest_alt_direction = 0u;
    i32 lowest_alt = 0;

    size_t reversed_direction = 0u;

    // this is an ocean tile !
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
                cell->freshwater_height = cell_total_height(flowed_to_cell) - (frwtr_m_t) cell->altitude + 1u;
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
static void freshwater_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    hexa_cell_t *flow_direction_tile = NULL;
    size_t nb_freshwatered_tiles = 0u;

    if ((cell->freshwater_height == 0u) && (cell->altitude > 0)) {
        return;
    }

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = (hexa_cell_t *) neighbors[i];
        nb_freshwatered_tiles += (tmp_cell->freshwater_height > 0u);
    }
    flow_direction_tile = (hexa_cell_t *) neighbors[cell->freshwater_direction];

    // happens when a freshwatered tile has a majority of freshwatered tiles around it.
    if (nb_freshwatered_tiles > (DIRECTIONS_NB / 2)) {
        hexa_cell_set_flag(cell, HEXAW_FLAG_MEANDERS);
    }

    // happens when a freshwatered tile is directed toward a significant drop (>100m)
    if (((flow_direction_tile->altitude * (flow_direction_tile->altitude > 0)) + FRESHWATER_WATERFALL_HEIGHT_THRESHOLD) < cell->altitude) {
        hexa_cell_set_flag(cell, HEXAW_FLAG_WATERFALLS);
    }

    // happens when a freshwatered tile is directed toward an ocean tile
    if (flow_direction_tile->altitude <= 0) {
        hexa_cell_set_flag(cell, HEXAW_FLAG_RIVER_MOUTH);
    }

    // happens when a freshwater tile has a significant depth (> 5m)
    if (cell->freshwater_height >= FRESHWATER_LAKE_DEPTH_THRESHOLD) {
        hexa_cell_set_flag(cell, HEXAW_FLAG_LAKE);
    }
}

// -------------------------------------------------------------------------------------------------
static i32 cell_total_height(hexa_cell_t *cell) {
    return ((i32) cell->altitude + (i32) cell->freshwater_height);
}

const layer_calls_t freshwater_layer_calls = {
        .draw_func          = &freshwater_draw,
        .seed_func          = &freshwater_seed,
        .automaton_func     = &freshwater_apply,
        .flag_gen_func      = &freshwater_flag_gen, 
        .automaton_iter     = ITERATION_NB_FRESHWATER,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};