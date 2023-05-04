
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
    Color inner_color = AS_RAYLIB_COLOR(COLOR_TREE_GREEN);
    u32 ocean_color = COLOR_CERULEAN;

    if (cell->altitude <= 0) {
        draw_hexagon(target_shape, ocean_color, 1.0f , DRAW_HEXAGON_FILL);
        return;
    }

    tile_color.a = (u8) (0xFF * cell->vegetation_cover);
    inner_color.a = (u8) (0xFF * cell->vegetation_lushness);
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

#if 1
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            tmp_tile = world->tiles[x] + y;

            water_rating = MAX(tmp_tile->precipitations, (tmp_tile->freshwater_height > 0u) * 0.5f);
            temperature_rating = 
                    NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, world->tiles[x][y].temperature / 2)
                    / NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, VEGETATION_TEMPERATURE_MEAN);
            terrain_rating = (1.0f - (0.25f * (f32) (
                      hexa_cell_has_flag(tmp_tile, HEXAW_FLAG_CANYONS) 
                    + hexa_cell_has_flag(tmp_tile, HEXAW_FLAG_MOUNTAIN)
                    + hexa_cell_has_flag(tmp_tile, HEXAW_FLAG_MEANDERS)
                    + hexa_cell_has_flag(tmp_tile, HEXAW_FLAG_LONG_COAST))));
            cloudiness_rating = tmp_tile->cloud_cover;

            tmp_tile->vegetation_cover = temperature_rating * water_rating * terrain_rating;
            tmp_tile->vegetation_lushness = temperature_rating * water_rating * cloudiness_rating;
        }
    }

#else
    ratio_t cloud_cover_rating = 0.0f;
    ratio_t temperature_rating = 0.0f;

    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            cloud_cover_rating = (world->tiles[x][y].cloud_cover * (world->tiles[x][y].freshwater_height == 0u)) + (world->tiles[x][y].freshwater_height > 0u);
            temperature_rating = 
                    NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, world->tiles[x][y].temperature / 2)
                    / NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, VEGETATION_TEMPERATURE_MEAN);

            world->tiles[x][y].vegetation_cover = 
                    cloud_cover_rating * temperature_rating 
                    * ((world->tiles[x][y].altitude > 0) || hexa_cell_has_flag(world->tiles[x] + y, HEXAW_FLAG_ISLES));
        }
    }
#endif
}

// -------------------------------------------------------------------------------------------------
static void vegetation_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
#if 1
#else
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    f32 vegetation_mean = 0.0f;
    size_t nb_cells_with_vegetation = 0u;

    if (((cell->altitude <= 0) && !(hexa_cell_has_flag(cell, HEXAW_FLAG_ISLES))) || (cell->vegetation_cover > VEGETATION_CUTOUT_THRESHOLD)) {
        return;
    }
    
    for (size_t i = 0 ; i < DIRECTIONS_NB; i++) {
        tmp_cell = (hexa_cell_t *) neighbors[i];

        vegetation_mean += tmp_cell->vegetation_cover;
        nb_cells_with_vegetation += (tmp_cell->vegetation_cover > VEGETATION_CUTOUT_THRESHOLD);
    }

    if (nb_cells_with_vegetation == 0u) {
        return;
    }
    vegetation_mean /= nb_cells_with_vegetation;

    cell->vegetation_cover = vegetation_mean 
                    * (NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, cell->temperature)
                    / NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, VEGETATION_TEMPERATURE_MEAN));
#endif
}

const layer_calls_t vegetation_layer_calls = {
        .draw_func          = &vegetation_draw,
        .seed_func          = &vegetation_seed,
        .automaton_func     = &vegetation_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_VEGETATION,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};