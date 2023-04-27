
#include "hexaworldcomponents.h"

#include <math.h>

#include <raylib.h>

#define ITERATION_NB_HUMIDITY (20u)    ///< number of automaton iteration for the humidity layer

static void humidity_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void humidity_seed(hexaworld_t *world);

static void humidity_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -- HUMIDITY -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void humidity_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color base_color = (Color) { 0x6e, 0xBA, 0xFF, 0xFF };
    
    base_color.a = (u8) (cell->humidity * 255u);
    DrawPoly(*((Vector2*) &target_shape->center), HEXAGON_SIDES_NB, target_shape->radius, 0.0f, base_color);
    DrawCircleV(
            *((Vector2 *) &(target_shape->center)), 
            (2*target_shape->radius/3) * cell->precipitations, 
            (Color) { 0xD8, 0xD8, 0xD8, 0xFF }
    );
    if (cell->altitude > 0) {
        DrawPolyLines(*((Vector2*) &target_shape->center), HEXAGON_SIDES_NB, 5*target_shape->radius/6, 0.0f, GRAY);
    }
}

// -------------------------------------------------------------------------------------------------
static void humidity_seed(hexaworld_t *world) {
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].humidity = (f32) (world->tiles[x][y].altitude <= 0);
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void humidity_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    f32 source_humidity = 0.0f;
    f32 inv_angle_wind = 0.0f;
    size_t wind_source_cell = 0u;

    if (cell->altitude <= 0) {
        return;
    }

    inv_angle_wind = (fmod(cell->winds_vector.angle + PI, PI_T_2)) / (PI_T_2);
    wind_source_cell = (size_t) (inv_angle_wind * (f32) DIRECTIONS_NB) % DIRECTIONS_NB;
    source_humidity = ((hexa_cell_t *) neighbors[wind_source_cell])->humidity;

    if (float_equal(source_humidity, 0.0f, 1u)) {
        return;
    }
    
    cell->humidity = source_humidity * (cell->winds_vector.magnitude);
    cell->precipitations = source_humidity - cell->humidity;
}

const layer_calls_t humidity_layer_calls = {
        .draw_func          = &humidity_draw,
        .seed_func          = &humidity_seed,
        .automaton_func     = &humidity_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_HUMIDITY,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};