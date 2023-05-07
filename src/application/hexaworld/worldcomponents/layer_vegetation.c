
#include "hexaworldcomponents.h"

#include <math.h>

#include <raylib.h>

#include <colorpalette.h>

#define ITERATION_NB_VEGETATION (10u)    ///< number of automaton iteration for the vegetation layer

static f32 get_temperature_rating(hexa_cell_t *cell);
static f32 get_terrain_rating(hexa_cell_t *cell);
static f32 get_water_rating(hexa_cell_t *cell);
static f32 get_cloudiness_rating(hexa_cell_t *cell);

// -------------------------------------------------------------------------------------------------
// -- VEGETATION -----------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void vegetation_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = AS_RAYLIB_COLOR(COLOR_LEAFY_GREEN);
    Color inner_color = AS_RAYLIB_COLOR(COLOR_TREE_GREEN);
    u32 ocean_color = COLOR_CERULEAN;

    if (cell->altitude <= 0) {
        draw_hexagon(target_shape, ocean_color, 1.0f , DRAW_HEXAGON_FILL);
        return;
    }

    tile_color.a = (u8) (0xFF * cell->vegetation_cover);
    inner_color.a = (u8) (0xFF * cell->vegetation_trees);
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(tile_color), 1.0f , DRAW_HEXAGON_FILL);
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(inner_color), 0.5f , DRAW_HEXAGON_FILL);
    
    tile_color.a = (cell->vegetation_cover > VEGETATION_CUTOUT_THRESHOLD) * 0xFF;
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(tile_color), 0.83f , DRAW_HEXAGON_LINES);
}

// -------------------------------------------------------------------------------------------------
static void vegetation_seed(hexaworld_t *world) {
    // more water (precipitations & freshwater) means more overall vegetation
    ratio_t water_rating = 0.0f;
    // there is a precise temperature range needed by the plants to grow
    ratio_t temperature_rating = 0.0f;
    // terrain features can give penalities to the vegetation growth
    ratio_t terrain_rating = 0.0f;
    // cloud cover can benefit some plants
    ratio_t cloudiness_rating = 0.0f;

    hexa_cell_t *tmp_tile = NULL;
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            tmp_tile = world->tiles[x] + y;

            if (tmp_tile->altitude <= 0) {
                continue;
            }

            temperature_rating = get_temperature_rating(tmp_tile);
            cloudiness_rating = get_cloudiness_rating(tmp_tile);

            tmp_tile->vegetation_cover = temperature_rating * cloudiness_rating;
            tmp_tile->vegetation_trees = 
                    (tmp_tile->vegetation_cover > VEGETATION_CUTOUT_THRESHOLD)
                    * ((tmp_tile->freshwater_height > 0) || tmp_tile->precipitations > 0)
                    * MIN(temperature_rating + (tmp_tile->vegetation_cover * VEGETATION_COVER_HELP_FOR_TREES), 1.0f);
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void vegetation_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;
    hexa_cell_t *tmp_cell = NULL;
    f32 max_veg_cover = 0.0f;
    f32 mean_veg_trees = 0.0f;
    f32 temperature_rating = 0.0f;

    if (cell->altitude <= 0) {
        return;
    }

    temperature_rating = get_temperature_rating(cell);

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = (hexa_cell_t *) neighbors[i];

        if (tmp_cell->vegetation_cover > max_veg_cover) {
            max_veg_cover = tmp_cell->vegetation_cover;
        }
        mean_veg_trees += tmp_cell->vegetation_trees;
    }
    mean_veg_trees /= (f32) DIRECTIONS_NB;

    cell->vegetation_cover = MAX(max_veg_cover * VEGETATION_COVER_DIFFUSION_FACTOR * temperature_rating, cell->vegetation_cover);
    if (cell->vegetation_trees < VEGETATION_CUTOUT_THRESHOLD) {
        cell->vegetation_trees = sigmoid(mean_veg_trees - VEGETATION_TREES_PROPAGATION_OFFSET, VEGETATION_TREES_PROPAGATION_WEIGHT);
    }
}

// -------------------------------------------------------------------------------------------------
static f32 get_temperature_rating(hexa_cell_t *cell) {
    return normal_distribution((f32) (cell->temperature / 2), VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI)
            / normal_distribution(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI);
}

// -------------------------------------------------------------------------------------------------
static f32 get_terrain_rating(hexa_cell_t *cell) {
    return (1.0f - (0.25f * (f32) (
              hexa_cell_has_flag(cell, HEXAW_FLAG_CANYONS) 
            + hexa_cell_has_flag(cell, HEXAW_FLAG_MOUNTAIN)
            + hexa_cell_has_flag(cell, HEXAW_FLAG_MEANDERS)
            + hexa_cell_has_flag(cell, HEXAW_FLAG_LONG_COAST))));
}

// -------------------------------------------------------------------------------------------------
static f32 get_water_rating(hexa_cell_t *cell) {
    return MAX(cell->precipitations, (cell->freshwater_height > 0u) * 0.8f);
}

// -------------------------------------------------------------------------------------------------
static f32 get_cloudiness_rating(hexa_cell_t *cell) {
    return cell->cloud_cover;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

const layer_calls_t vegetation_layer_calls = {
        .draw_func          = &vegetation_draw,
        .seed_func          = &vegetation_seed,
        .automaton_func     = &vegetation_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_VEGETATION,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};