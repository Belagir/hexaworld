
#include "hexaworldcomponents.h"

#include <math.h>

#include <raylib.h>

#include <colorpalette.h>

#define ITERATION_NB_CLOUD_COVER (20u)    ///< number of automaton iteration for the cloud cover layer

// -------------------------------------------------------------------------------------------------
// -- CLOUD COVER -----------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void dcloud_cover_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color base_color = AS_RAYLIB_COLOR(COLOR_CERULEAN);
    
    base_color.a = (u8) (cell->cloud_cover * 255u);
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(base_color), 1.0f, DRAW_HEXAGON_FILL);
    DrawCircleV(
            *((Vector2 *) &(target_shape->center)), 
            (2*target_shape->radius/3) * cell->precipitations, 
            AS_RAYLIB_COLOR(COLOR_GRAY)
    );
    if (cell->altitude > 0) {
        draw_hexagon(target_shape, COLOR_LEATHER, 0.83f, DRAW_HEXAGON_LINES);
    }
}

// -------------------------------------------------------------------------------------------------
static void dcloud_cover_seed(hexaworld_t *world) {
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].cloud_cover = (f32) (world->tiles[x][y].altitude <= 0);
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void dcloud_cover_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    f32 source_cloud_cover = 0.0f;
    f32 inv_angle_wind = 0.0f;
    size_t wind_source_cell = 0u;

    if (cell->altitude <= 0) {
        return;
    }

    inv_angle_wind = (fmod(cell->winds_vector.angle + PI, PI_T_2)) / (PI_T_2);
    wind_source_cell = (size_t) (inv_angle_wind * (f32) DIRECTIONS_NB) % DIRECTIONS_NB;
    source_cloud_cover = ((hexa_cell_t *) neighbors[wind_source_cell])->cloud_cover;

    if (float_equal(source_cloud_cover, 0.0f, 1u)) {
        return;
    }
    
    cell->cloud_cover = source_cloud_cover * (cell->winds_vector.magnitude);
    cell->precipitations = source_cloud_cover - cell->cloud_cover;
}

const layer_calls_t cloud_cover_layer_calls = {
        .draw_func          = &dcloud_cover_draw,
        .seed_func          = &dcloud_cover_seed,
        .automaton_func     = &dcloud_cover_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_CLOUD_COVER,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};