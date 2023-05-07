
#include "hexaworldcomponents.h"

#include <stdlib.h>

#include <raylib.h>
#include <colorpalette.h>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

#define ITERATION_NB_WHOLE_WORLD (0u)

#define WHOLE_WORLD_RANDOM_GEN_ISLE_STEP_ANGLE (4u)
#define WHOLE_WORLD_RANDOM_GEN_ISLE_STEP_RADIUS (2u)
#define WHOLE_WORLD_ISLES_MAX_NB (2u)

#define WHOLE_WORLD_RANDOM_GEN_FOREST_STEP_ANGLE (12u)
#define WHOLE_WORLD_RANDOM_GEN_FOREST_STEP_RADIUS (4u)

#define WHOLE_WORLD_AUTOMATIC_SNOW_TEMP (-20)

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Determines the color of a tile if it were an ocean tile.
 * 
 * @param[in] cell cell containing the information about the tile
 * @return Color ocean color
 */
static Color color_ocean_tile(hexa_cell_t *cell);

/**
 * @brief Determines the color of a tile if it were an earth tile.
 * 
 * @param[in] cell cell containing the information about the tile
 * @return Color earth color
 */
static Color color_earth_tile(hexa_cell_t *cell);

/**
 * @brief Determines the snow accent color of a tile.
 * 
 * @param[in] cell cell containing the information about the tile
 * @return Color snow color
 */
static Color color_snowy_tile(hexa_cell_t *cell);

/**
 * @brief Determines the vegetation accent color of a tile.
 * 
 * @param[in] cell cell containing the information about the tile
 * @return Color vegetation color
 */
static Color color_veget_tile(hexa_cell_t *cell);

/**
 * @brief  Determines the trees accent color of a tile.
 * 
 * @param[in] cell cell containing the information about the tile
 * @return Color trees color
 */
static Color color_trees_tile(hexa_cell_t *cell);

/**
 * @brief Draws the potential mountain or canyon present on a tile.
 * 
 * @param[in] cell cell containing the information about the tile
 * @param[in] target_shape destination hexagon shape
 */
static void draw_mountains_canyon(hexa_cell_t *cell, hexagon_shape_t *target_shape);

/**
 * @brief Draws the potential isles present on a tile.
 * 
 * @param[in] cell cell containing the information about the tile
 * @param[in] target_shape destination hexagon shape
 */
static void draw_isles(hexa_cell_t *cell, hexagon_shape_t *target_shape);

/**
 * @brief Draws the potential freshwater feature present on a tile.
 * 
 * @param[in] cell cell containing the information about the tile
 * @param[in] target_shape destination hexagon shape
 */
static void draw_freshwater(hexa_cell_t *cell, hexagon_shape_t *target_shape);

/**
 * @brief 
 * 
 * @param cell 
 * @param target_shape 
 */
static void draw_forests(hexa_cell_t *cell, hexagon_shape_t *target_shape);

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
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(color_snowy_tile(cell)), 1.0f , DRAW_HEXAGON_FILL);

    if  (cell->altitude > 0) {
        draw_hexagon(target_shape, FROM_RAYLIB_COLOR(color_veget_tile(cell)), 1.0f , DRAW_HEXAGON_FILL);
    }

    draw_isles(cell, target_shape);
    draw_forests(cell, target_shape);
    draw_mountains_canyon(cell, target_shape);
    draw_freshwater(cell, target_shape);

}

// -------------------------------------------------------------------------------------------------
static Color color_ocean_tile(hexa_cell_t *cell) {
    Color tile_color = AS_RAYLIB_COLOR(COLOR_DUSK_BLUE);

    if ((cell->temperature <= (WHOLE_WORLD_AUTOMATIC_SNOW_TEMP / 2)) && (cell->altitude > (ALTITUDE_MIN / 4)) || (cell->temperature <= WHOLE_WORLD_AUTOMATIC_SNOW_TEMP)) {
        tile_color = AS_RAYLIB_COLOR(COLOR_ICE_BLUE);
    } else if (cell->altitude > (WHOLE_WORLD_OCEAN_REEF_CUTOUT * ALTITUDE_MIN)) {
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
static Color color_snowy_tile(hexa_cell_t *cell) {
    Color tile_color = AS_RAYLIB_COLOR(COLOR_ICE_BLUE);

    if (cell->temperature <= WHOLE_WORLD_AUTOMATIC_SNOW_TEMP) {
        tile_color.a = 0xFF;
    } else if ((cell->temperature > 0) || (cell->altitude <= 0)) {
        tile_color.a = 0x00;
    } else {
        tile_color.a = (1.0f - ((f32) (cell->temperature - TEMPERATURE_MIN) / (f32) -TEMPERATURE_MIN)) * cell->cloud_cover * 0xFF;
    }
    
    return tile_color;
}

static Color color_veget_tile(hexa_cell_t *cell) {
    Color tile_color = AS_RAYLIB_COLOR(COLOR_LEAFY_GREEN);

    tile_color.a = cell->vegetation_cover * 0xFF;
    
    return tile_color;
}

// -------------------------------------------------------------------------------------------------
static Color color_trees_tile(hexa_cell_t *cell) {
    Color arid_color = AS_RAYLIB_COLOR(COLOR_LEAF);
    Color lush_color = AS_RAYLIB_COLOR(COLOR_TREE_GREEN);

    Color tile_color = (Color) {
            .r = arid_color.r + (lush_color.r - arid_color.r) * (cell->vegetation_cover),
            .g = arid_color.g + (lush_color.g - arid_color.g) * (cell->vegetation_cover),
            .b = arid_color.b + (lush_color.b - arid_color.b) * (cell->vegetation_cover),
            .a = 0xFF * cell->vegetation_trees,
    };

    return tile_color;

}

// -------------------------------------------------------------------------------------------------
static void draw_mountains_canyon(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color feature_color = { 0u };

    if ((!hexa_cell_has_flag(cell, HEXAW_FLAG_MOUNTAIN)) && (!hexa_cell_has_flag(cell, HEXAW_FLAG_CANYONS))) {
        return;
    }

    feature_color = AS_RAYLIB_COLOR(COLOR_LEATHER);
    feature_color.a = 0x8F + (u8) ((f32) 0x70 * ((f32) cell->altitude / (f32) ALTITUDE_MAX));

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_MOUNTAIN)) {
        draw_hexagon(target_shape, FROM_RAYLIB_COLOR(feature_color), 0.66f, DRAW_HEXAGON_FILL);
        if (cell->temperature <= 0) {
            draw_hexagon(target_shape, COLOR_ICE_BLUE, 0.3f , DRAW_HEXAGON_FILL);
        }
    }
    feature_color.a = 0xE0;
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(feature_color), 0.66f, DRAW_HEXAGON_LINES);
}

// -------------------------------------------------------------------------------------------------
static void draw_isles(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = AS_RAYLIB_COLOR(COLOR_LEAFY_GREEN);
    vector_2d_polar_t random_isle_pos = { 0u };
    u32 random_nb_isles = 0u;
    hexagon_shape_t isle_shape = { 0u };

    if (!hexa_cell_has_flag(cell, HEXAW_FLAG_ISLES)) {
        return;
    }

    isle_shape.radius = target_shape->radius * 0.4f;

    tile_color = color_veget_tile(cell);

    random_nb_isles = (u32) ((i32) (rand() % WHOLE_WORLD_ISLES_MAX_NB));
    for (size_t i = 0u ; i < random_nb_isles ; i++) {
        random_isle_pos.angle = ((f32) (rand() % WHOLE_WORLD_RANDOM_GEN_ISLE_STEP_ANGLE) / (f32) WHOLE_WORLD_RANDOM_GEN_ISLE_STEP_ANGLE) * PI_T_2;
        random_isle_pos.magnitude = (f32) (rand() % WHOLE_WORLD_RANDOM_GEN_ISLE_STEP_RADIUS) / (f32) WHOLE_WORLD_RANDOM_GEN_ISLE_STEP_RADIUS;
        random_isle_pos.magnitude = (random_isle_pos.magnitude * (target_shape->radius - isle_shape.radius));

        isle_shape.center = vector2d_polar_to_cartesian(random_isle_pos);
        isle_shape.center.v += target_shape->center.v;
        isle_shape.center.w += target_shape->center.w;

        draw_hexagon(&isle_shape, FROM_RAYLIB_COLOR(color_earth_tile(cell)), 1.0f, DRAW_HEXAGON_FILL);
        draw_hexagon(&isle_shape, FROM_RAYLIB_COLOR(color_snowy_tile(cell)), 1.0f, DRAW_HEXAGON_FILL);
        draw_hexagon(&isle_shape, FROM_RAYLIB_COLOR(tile_color), 1.0f, DRAW_HEXAGON_FILL);
    }
}

// -------------------------------------------------------------------------------------------------
static void draw_freshwater(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color color_line = AS_RAYLIB_COLOR(COLOR_CERULEAN);
    Color color_ice = AS_RAYLIB_COLOR(COLOR_ICE_BLUE);
    vector_2d_cartesian_t water_end = { 0u };
    vector_2d_cartesian_t water_start = { 0u };

    if (cell->freshwater_height == 0u) {
        return;
    }

    if (cell->temperature < -10) {
         color_ice.a = 0xFF;
    } else if (cell->temperature <= 0) {
        color_ice.a = (u8) (0xFF * ((f32) (cell->temperature + 10) / 10.0));
    } else {
        color_ice.a = 0x00;
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

// -------------------------------------------------------------------------------------------------
static void draw_forests(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color forest_color = color_trees_tile(cell);

    if (cell->vegetation_trees < VEGETATION_CUTOUT_THRESHOLD) {
        return;
    }

    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(forest_color), 0.75f, DRAW_HEXAGON_FILL);
    draw_hexagon(target_shape, COLOR_TREE_GREEN, 0.75f, DRAW_HEXAGON_LINES);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------


const layer_calls_t whole_world_layer_calls = {
        .draw_func          = &whole_world_draw,
        .seed_func          = NULL,
        .automaton_func     = NULL,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_WHOLE_WORLD,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};
