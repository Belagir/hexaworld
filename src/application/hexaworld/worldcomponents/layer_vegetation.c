
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
    inner_color.a = (u8) (0xFF * cell->vegetation_volume);
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(tile_color), 1.0f , DRAW_HEXAGON_FILL);
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(inner_color), 0.5f , DRAW_HEXAGON_FILL);
    
    tile_color.a = (cell->vegetation_cover > VEGETATION_CUTOUT_THRESHOLD) * 0xFF;
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(tile_color), 0.83f , DRAW_HEXAGON_LINES);
}

// -------------------------------------------------------------------------------------------------
static void vegetation_seed(hexaworld_t *world) {
#if 1
#else
    // more water (precipitations & freshwater) means more overall vegetation
    ratio_t water_rating = 0.0f;
    // there is a precise temperature range needed by the plants to grow
    ratio_t temperature_rating = 0.0f;
    // terrain features can give penalities to the vegetation growth
    ratio_t terrain_rating = 0.0f;
    // cloud cover can benefit some plants
    ratio_t cloudiness_rating = 0.0f;

    hexa_cell_t *tmp_tile = NULL;

    // only abiotic factors for seeding
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            tmp_tile = world->tiles[x] + y;

            water_rating = get_water_rating(tmp_tile);
            temperature_rating = get_temperature_rating(tmp_tile);
            terrain_rating = get_terrain_rating(tmp_tile);
            cloudiness_rating = get_cloudiness_rating(tmp_tile);

            tmp_tile->vegetation_cover = temperature_rating * (water_rating + cloudiness_rating) / 2.0f;
            tmp_tile->vegetation_volume = tmp_tile->vegetation_cover * temperature_rating * (water_rating + terrain_rating) / 2.0f;
        }
    }
 #endif
}

// -------------------------------------------------------------------------------------------------
static void vegetation_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;
#if 1
#else
    hexa_cell_t *tmp_cell = NULL;
    f32 max_veg_volume = 0.0f;

    f32 temperature_rating = 0.0f;
    f32 terrain_rating = 0.0f;
    f32 shade_rating = 0.0f;
    f32 water_rating = 0.0f;

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = (hexa_cell_t *) neighbors[i];

        if (tmp_cell->vegetation_volume > max_veg_volume) {
            max_veg_volume = tmp_cell->vegetation_volume;
        }
    }

    temperature_rating = get_temperature_rating(cell);
    shade_rating = MIN(get_cloudiness_rating(cell) + cell->vegetation_volume, 1.0f);
    water_rating = get_water_rating(cell);
    terrain_rating = get_terrain_rating(cell);

    cell->vegetation_cover = temperature_rating * MIN((water_rating + shade_rating) / 2.0f + max_veg_volume, 1.0f);
    cell->vegetation_volume = cell->vegetation_cover * temperature_rating * (water_rating + terrain_rating) / 2.0f;
#endif
}

// -------------------------------------------------------------------------------------------------
static f32 get_temperature_rating(hexa_cell_t *cell) {
    return NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, cell->temperature / 2)
            / NORMAL_DISTRIBUTION(VEGETATION_TEMPERATURE_MEAN, VEGETATION_TEMPERATURE_VARI, VEGETATION_TEMPERATURE_MEAN);
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