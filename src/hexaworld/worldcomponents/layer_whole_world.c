
#include "hexaworldcomponents.h"

#include <stdlib.h>

#include <raylib.h>
#include <colorpalette.h>

#define ITERATION_NB_WHOLE_WORLD (0u)

static Color color_ocean_tile(hexa_cell_t *cell);
static Color color_earth_tile(hexa_cell_t *cell);

static void draw_vegetation(hexa_cell_t *cell, hexagon_shape_t *target_shape);
static void draw_snow(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void draw_mountains_canyon(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void draw_freshwater(hexa_cell_t *cell, hexagon_shape_t *target_shape);

// -------------------------------------------------------------------------------------------------
// -- WHOLE WORLD ----------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void whole_world_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = { 0u };

    if (cell->altitude > 0) {
        tile_color = color_earth_tile(cell);
    } else {
        tile_color = color_ocean_tile(cell);
    }

    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(tile_color), 1.0f , DRAW_HEXAGON_FILL);

    if  (cell->altitude > 0) {
        draw_vegetation(cell, target_shape);
    }
    draw_snow(cell, target_shape);

    draw_mountains_canyon(cell, target_shape);
    draw_freshwater(cell, target_shape);
}

// -------------------------------------------------------------------------------------------------
static Color color_ocean_tile(hexa_cell_t *cell) {
    Color tile_color = AS_RAYLIB_COLOR(COLOR_DUSK_BLUE);

    if (cell->altitude > (WHOLE_WORLD_OCEAN_REEF_CUTOUT * ALTITUDE_MIN)) {
        tile_color = AS_RAYLIB_COLOR(COLOR_AZURE);
    } else if (cell->altitude > (WHOLE_WORLD_OCEAN_ABYSS_CUTOUT * ALTITUDE_MIN)) {
        tile_color = AS_RAYLIB_COLOR(COLOR_CERULEAN);
    }

    return tile_color;
}

// -------------------------------------------------------------------------------------------------
static Color color_earth_tile(hexa_cell_t *cell) {
    Color rocky_color = AS_RAYLIB_COLOR(COLOR_TAUPE);
    Color sandy_color = AS_RAYLIB_COLOR(COLOR_WHEAT);

    Color tile_color = (Color) {
            .r = rocky_color.r + (sandy_color.r - rocky_color.r) * ((f32) (cell->temperature - TEMPERATURE_MIN) / (f32) TEMPERATURE_RANGE),
            .g = rocky_color.g + (sandy_color.g - rocky_color.g) * ((f32) (cell->temperature - TEMPERATURE_MIN) / (f32) TEMPERATURE_RANGE),
            .b = rocky_color.b + (sandy_color.b - rocky_color.b) * ((f32) (cell->temperature - TEMPERATURE_MIN) / (f32) TEMPERATURE_RANGE),
            .a = 0xFF
    };

    return tile_color;
}

// -------------------------------------------------------------------------------------------------
static void draw_vegetation(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = AS_RAYLIB_COLOR(COLOR_LEAFY_GREEN);

    tile_color.a = cell->vegetation_cover * 0xFF;
    
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(tile_color), 1.0f , DRAW_HEXAGON_FILL);
}

// -------------------------------------------------------------------------------------------------
static void draw_snow(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = AS_RAYLIB_COLOR(COLOR_ICE_BLUE);

    if ((cell->temperature > 0) || (cell->altitude <= 0)) {
        return;
    }

    tile_color.a = ((f32) (cell->temperature - TEMPERATURE_MIN) / (f32) TEMPERATURE_RANGE) * 0xFF;
    
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(tile_color), 1.0f , DRAW_HEXAGON_FILL);
}

// -------------------------------------------------------------------------------------------------
static void draw_mountains_canyon(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color feature_color = { 0u };

    if ((!hexa_cell_has_flag(cell, HEXAW_FLAG_MOUNTAIN)) && (!hexa_cell_has_flag(cell, HEXAW_FLAG_CANYONS))) {
        return;
    }

    feature_color = AS_RAYLIB_COLOR(COLOR_LEATHER);
    feature_color.a = 0x7F;

    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(feature_color), 0.66f , hexa_cell_has_flag(cell, HEXAW_FLAG_MOUNTAIN) ? DRAW_HEXAGON_FILL : DRAW_HEXAGON_LINES);
}

// -------------------------------------------------------------------------------------------------
static void draw_freshwater(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color color_line = AS_RAYLIB_COLOR(COLOR_CERULEAN);
    vector_2d_cartesian_t water_end = { 0u };
    vector_2d_cartesian_t water_start = { 0u };

    if (cell->freshwater_height == 0u) {
        return;
    }

    if (cell->temperature <= 0) {
        color_line = AS_RAYLIB_COLOR(COLOR_AZURE);
    }
    
    water_end = vector2d_polar_to_cartesian((vector_2d_polar_t) { 
            .angle = ((f32) cell->freshwater_direction / (f32) DIRECTIONS_NB) * PI_T_2,
            .magnitude = 1.0f
    });

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_LAKE)) {
        draw_hexagon(target_shape, FROM_RAYLIB_COLOR(color_line), 0.66f , DRAW_HEXAGON_FILL);

    }

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        if (cell->freshwater_sources_directions & (0x1 << i)) {
            water_start = vector2d_polar_to_cartesian((vector_2d_polar_t) { 
                    .angle = ((f32) i / (f32) DIRECTIONS_NB) * PI_T_2,
                    .magnitude = 1.0f
            });

            DrawLineBezierQuad(
                    (Vector2) {
                            .x = target_shape->center.v + water_start.v * target_shape->radius, 
                            .y = target_shape->center.w + water_start.w * target_shape->radius },
                    (Vector2) {
                            .x = target_shape->center.v + water_end.v * target_shape->radius, 
                            .y = target_shape->center.w + water_end.w * target_shape->radius },
                    (Vector2) {
                            .x = target_shape->center.v, 
                            .y = target_shape->center.w },
                    
                    target_shape->radius / 5.0f, 
                    color_line
            );
        }
    }
}

const layer_calls_t whole_world_layer_calls = {
        .draw_func          = &whole_world_draw,
        .seed_func          = NULL,
        .automaton_func     = NULL,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_WHOLE_WORLD,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};
