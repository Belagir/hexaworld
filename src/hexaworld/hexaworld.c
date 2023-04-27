/**
 * @file hexaworld.c
 * @author gabriel 
 * @brief definition file for the hexaworld module.
 * @version 0.1
 * @date 2023-04-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "hexaworld.h"

#include <stdlib.h>
#include <math.h>
#include <raylib.h>

#include <cellotomaton.h>

#include "worldcomponents/hexaworldcomponents.h"

// -------------------------------------------------------------------------------------------------
// ---- FILE CONSTANTS -----------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// ------------ GENERATION TWEAKS ------------------------------------------------------------------
//
// mountains length             : ITERATION_NB_TELLURIC
// mountains / rifts density    : TELLURIC_VECTOR_DIRECTIONS_NB and TELLURIC_VECTOR_SEEDING_INV_CHANCE
// continents size              : LANDMASS_SEEDING_CHANCE and ITERATION_NB_LANDMASS
//
// -------------------------------------------------------------------------------------------------

#define PI_T_2 ((2.0f) * (PI))    ///< 2 times pi

#define ITERATION_NB_TELLURIC (2u)    ///< number of automaton iteration for the telluric layer
#define ITERATION_NB_LANDMASS (6u)    ///< number of automaton iteration for the landmass layer
#define ITERATION_NB_ALTITUDE (10u)   ///< number of automaton iteration for the altitude layer
#define ITERATION_NB_WINDS (1u)       ///< number of automaton iteration for the winds layer
#define ITERATION_NB_HUMIDITY (20u)    ///< number of automaton iteration for the humidity layer

#define TELLURIC_VECTOR_SEEDING_INV_CHANCE (0x40)  ///< the greater, the bigger the chance a telluric tile is NOT seeded.
#define TELLURIC_VECTOR_DIRECTIONS_NB (32)     ///< number of possible directions for a telluric vector
#define TELLURIC_VECTOR_UNIT_ANGLE ((PI_T_2) / (TELLURIC_VECTOR_DIRECTIONS_NB))      ///< telluric vector minimum angle 

#define LANDMASS_SEEDING_CHANCE (0x03)    ///< the greater, the bigger the chance a land tile is seeded.

#define ALTITUDE_MAX (3000)  ///< maximum altitude, in meters
#define ALTITUDE_MIN (-2000)  ///< minimum altitude, in meters

#define WINDS_VECTOR_DIRECTIONS_NB (32)
#define WINDS_VECTOR_UNIT_ANGLE ((PI_T_2) / (WINDS_VECTOR_DIRECTIONS_NB))      ///< winds vector minimum angle 

// -------------------------------------------------------------------------------------------------
// ---- TYPE DEFINITIONS ---------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DECLARATIONS --------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// -- TELLURIC -------------------------------------------------------------------------------------

static void telluric_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void telluric_seed(hexaworld_t *world);

static void telluric_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

static void telluric_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -- LANDMASS -------------------------------------------------------------------------------------

static void landmass_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void landmass_seed(hexaworld_t *world);

static void landmass_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

static void landmass_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -- ALTITUDE -------------------------------------------------------------------------------------

static void altitude_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void altitude_seed(hexaworld_t *world);

static void altitude_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -- WINDS ----------------------------------------------------------------------------------------

static void winds_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void winds_seed(hexaworld_t *world);

static void winds_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -- HUMIDITY -------------------------------------------------------------------------------------

static void humidity_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void humidity_seed(hexaworld_t *world);

static void humidity_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Draw an empty hexagonal grid on the current raylib drawing context.
 * 
 * @param[in] world non-NULL pointer to some world data
 * @param[in] rectangle_target rectangle defined by the topleft coordinates and its sides' length (in pixels)
 */
static void hexaworld_draw_grid(hexaworld_t *world, f32 rectangle_target[4u]);

// -------------------------------------------------------------------------------------------------
// ---- STATIC DATA --------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Map of the layer number to its related drawing and generating calls.
 */
static const layer_calls_t hexaworld_layers_functions[HEXAW_LAYERS_NUMBER] = {
    /// telluric layer calls
    { &telluric_draw,        &telluric_seed,        &telluric_apply,        &telluric_flag_gen,        ITERATION_NB_TELLURIC, LAYER_GEN_ITERATE_RELATIVE },
    /// landmass layer calls
    { &landmass_draw,        &landmass_seed,        &landmass_apply,        &landmass_flag_gen,        ITERATION_NB_LANDMASS, LAYER_GEN_ITERATE_ABSOLUTE },
    /// altitude layer calls
    { &altitude_draw,        &altitude_seed,        &altitude_apply,        NULL,                      ITERATION_NB_ALTITUDE, LAYER_GEN_ITERATE_RELATIVE },
    /// wind layers calls
    { &winds_draw,           &winds_seed,           &winds_apply,           NULL,                      ITERATION_NB_WINDS,    LAYER_GEN_ITERATE_ABSOLUTE },
    /// humidity layer
    { &humidity_draw,        &humidity_seed,        &humidity_apply,        NULL,                      ITERATION_NB_HUMIDITY, LAYER_GEN_ITERATE_ABSOLUTE },
};

// -------------------------------------------------------------------------------------------------
// ---- HEADER FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
hexaworld_t *hexaworld_create_empty(size_t width, size_t height) {
    hexaworld_t *world = NULL;

    // overall data structure
    world = malloc(sizeof(*world));
    if (!world) {
        return NULL;
    }

    // tiles columns
    world->tiles = malloc(sizeof(*world->tiles) * width);
    if (!world->tiles) {
        return NULL;
    }

    // tiles
    for (size_t i = 0u ; i < width; i++) {
        world->tiles[i] = malloc(sizeof(*world->tiles[i]) * height);
        if (!world->tiles[i]) {
            return NULL;
        }
        for (size_t j = 0u ; j < height ; j++) {
            world->tiles[i][j] = (hexa_cell_t) { 0u };
        }
    }
    
    // cell automaton
    world->automaton = otomaton_create((void **) world->tiles, width, height, sizeof(**(world->tiles)));
    if (!world->automaton) {
        return NULL;
    }

    world->width = width;
    world->height = height;

    return world;
}

// -------------------------------------------------------------------------------------------------
void hexaworld_destroy(hexaworld_t **world) {

    for (size_t i = 0u ; i < (*world)->width; i++) {
        free((*world)->tiles[i]);
    }
    free((*world)->tiles);

    otomaton_destroy(&((*world)->automaton));

    (*world)->width = 0u;
    (*world)->height = 0u;

    free((*world));

    *world = NULL;
}

// -------------------------------------------------------------------------------------------------
void hexaworld_draw(hexaworld_t *world, hexaworld_layer_t layer, f32 rectangle_target[4u]) {
    layer_draw_function_t layer_function = NULL;
    hexagon_shape_t shape = { 0u };

    layer_function = hexaworld_layers_functions[layer].draw_func;

    // drawing each cell
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            shape = hexagon_position_in_rectangle(rectangle_target, x, y, world->width, world->height);
            layer_function(world->tiles[x] + y, &shape);
        }
    }

    hexaworld_draw_grid(world, rectangle_target);
}

// -------------------------------------------------------------------------------------------------
void hexaworld_genlayer(hexaworld_t *world, hexaworld_layer_t layer) {
    size_t iteration_number = 0u;

    hexaworld_layers_functions[layer].seed_func(world);

    iteration_number = hexaworld_layers_functions[layer].automaton_iter;
    if (hexaworld_layers_functions[layer].iteration_flavour == LAYER_GEN_ITERATE_RELATIVE) {
        iteration_number *= (world->width / 10);
    }

    // applying the overall generation function N times
    otomaton_apply(world->automaton, iteration_number, hexaworld_layers_functions[layer].automaton_func);

    // if the flag gneration function exists, apply it one time
    if (hexaworld_layers_functions[layer].flag_gen_func) {
        otomaton_apply(world->automaton, 1u, hexaworld_layers_functions[layer].flag_gen_func);
    }
}

// -------------------------------------------------------------------------------------------------
void hexaworld_raze(hexaworld_t *world) {
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y] = (hexa_cell_t) { 0u };
        }
    }
}

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// -- TELLURIC -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void telluric_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    vector_2d_cartesian_t translated_vec = { 0u };
    Color tile_color = WHITE;

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_TELLURIC_RIDGE)) {
        tile_color = ORANGE;
    } else if (hexa_cell_has_flag(cell, HEXAW_FLAG_TELLURIC_RIFT)) {
        tile_color = YELLOW;
    }

    DrawPoly(*((Vector2*) (&target_shape->center)), HEXAGON_SIDES_NB, target_shape->radius, 0.0f, tile_color);

    translated_vec = vector2d_polar_to_cartesian(cell->telluric_vector);
    DrawLineV(
            *((Vector2*) (&target_shape->center)), 
            (Vector2) { 
                    .x = target_shape->center.v + translated_vec.v * target_shape->radius, 
                    .y = target_shape->center.w + translated_vec.w * target_shape->radius },
            RED
    );
}

// -------------------------------------------------------------------------------------------------
static void telluric_seed(hexaworld_t *world) {
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            if ((rand() % TELLURIC_VECTOR_SEEDING_INV_CHANCE) == 0) {
                world->tiles[x][y].telluric_vector = (vector_2d_polar_t) {
                        .angle = (rand() % TELLURIC_VECTOR_DIRECTIONS_NB) * TELLURIC_VECTOR_UNIT_ANGLE,
                        .magnitude = 1.0f
                };
            } else {
                world->tiles[x][y].telluric_vector = (vector_2d_polar_t) { 
                        .angle = 0.0f,
                        .magnitude = 0.0f
                };
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void telluric_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    u32 angle_counter[TELLURIC_VECTOR_DIRECTIONS_NB] = { 0u };
    hexa_cell_t *tmp_cell = NULL;
    size_t index_of_most_present = 0u;
    u32 one_neighbor_is_set = 0u;
    size_t angle_index = 0u;

    // the objective is to set the telluric vector's direction to the most represented direction around it
    // to make the seeds grow into zones. Once a cell's direction is set, it cannot change. We use the 
    // magnitude as a flag to know if a vector is set, because all telluric vectors must have a magnitude 
    // of 1.

    // if the current cell already has a direction, nothing is to be done
    if (!float_equal(cell->telluric_vector.magnitude, 0.0f, 1u)) {
        return;
    }

    // computing the most represented angle 
    for (size_t i = 0u ; i < DIRECTIONS_NB; i++) {
        // capturing the neighboring cell
        tmp_cell = (hexa_cell_t *) neighbors[i];
        // if the neighboring cell is unset, we ignore it
        if (float_equal(tmp_cell->telluric_vector.magnitude, 0.0f, 1u)) {
            continue;
        }
        
        // we need to know if at least one of the neighbor propagated its value to our cell
        one_neighbor_is_set = 1u;

        // counting the number of times the angle is encountered
        angle_index = (size_t) (tmp_cell->telluric_vector.angle / TELLURIC_VECTOR_UNIT_ANGLE);
        angle_counter[angle_index] += 1u;

        // remembering the most represented angle
        if (angle_counter[angle_index] > angle_counter[index_of_most_present]) {
            index_of_most_present = angle_index;
        }
    }
    
    if (one_neighbor_is_set) {
        cell->telluric_vector.angle = index_of_most_present * TELLURIC_VECTOR_UNIT_ANGLE;
        cell->telluric_vector.magnitude = 1.0f;
    }
}

// -------------------------------------------------------------------------------------------------
static void telluric_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *pushed_against_cell = NULL;
    size_t pushed_against_cell_index = 0u;

    hexa_cell_t *pushed_from_cell = NULL;
    size_t pushed_from_cell_index = 0u;

    pushed_against_cell_index = (size_t) (((cell->telluric_vector.angle / (PI_T_2))) * DIRECTIONS_NB);
    pushed_against_cell = neighbors[pushed_against_cell_index];

    pushed_from_cell_index = (size_t) (((fmod(cell->telluric_vector.angle+PI, (PI_T_2)) / (PI_T_2))) * DIRECTIONS_NB);
    pushed_from_cell = neighbors[pushed_from_cell_index];

    if (!float_equal(cell->telluric_vector.angle, pushed_against_cell->telluric_vector.angle, 1u)) {
        hexa_cell_set_flag(cell, HEXAW_FLAG_TELLURIC_RIDGE);
    } else if (!float_equal(cell->telluric_vector.angle, pushed_from_cell->telluric_vector.angle, 1u)) {
        hexa_cell_set_flag(cell, HEXAW_FLAG_TELLURIC_RIFT);
    }
}

// -------------------------------------------------------------------------------------------------
// -- LANDMASS -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void landmass_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = (Color) { 0x20, 0x79, 0xF7, 0xFF };

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_UNDERWATER_CANYONS)) {
        tile_color = (Color) { 0x21, 0x36, 0x8F, 0xFF };
    } else if (hexa_cell_has_flag(cell, HEXAW_FLAG_MOUNTAIN)) {
        tile_color = (Color) { 0x90, 0x5F, 0x07, 0xFF };
    } else if (hexa_cell_has_flag(cell, HEXAW_FLAG_CANYONS)) {
        tile_color = (Color) { 0x93, 0x7E, 0x58, 0xFF };
        
    } else if (cell->altitude > 0) {
        tile_color = (Color) { 0xC8, 0x9B, 0x49, 0xFF };
    }


    DrawPoly(*((Vector2*) &target_shape->center), HEXAGON_SIDES_NB, target_shape->radius, 0.0f, tile_color);

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_ISLES)) {
        DrawCircleV(*((Vector2*) &target_shape->center), target_shape->radius/2, (Color) { 0xC8, 0x9B, 0x49, 0xFF });   
    }
}

// -------------------------------------------------------------------------------------------------
static void landmass_seed(hexaworld_t *world) {
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            if (hexa_cell_has_flag(world->tiles[x] + y, HEXAW_FLAG_TELLURIC_RIDGE)) {
                world->tiles[x][y].altitude = 1;
            } else if (!hexa_cell_has_flag(world->tiles[x] + y, HEXAW_FLAG_TELLURIC_RIFT)){
                world->tiles[x][y].altitude = (rand() & LANDMASS_SEEDING_CHANCE) != 0;
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void landmass_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    u32 counter = 0u;

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        counter += ((hexa_cell_t *) neighbors[i])->altitude;
    }

    cell->altitude = (counter > (DIRECTIONS_NB / 2));
}

// -------------------------------------------------------------------------------------------------
static void landmass_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    const u32 is_above_sea_level = cell->altitude;
    const u32 is_a_ridge = hexa_cell_has_flag(cell, HEXAW_FLAG_TELLURIC_RIDGE);
    const u32 is_a_rift = hexa_cell_has_flag(cell, HEXAW_FLAG_TELLURIC_RIFT);

    if (is_above_sea_level) {
        if (is_a_ridge) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_MOUNTAIN);
        } else if (is_a_rift) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_CANYONS);
        }
    } else {
        if (is_a_ridge) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_ISLES);
        } else if (is_a_rift) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_UNDERWATER_CANYONS);
        }
    }
}

// -------------------------------------------------------------------------------------------------
// -- ALTITUDE -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void altitude_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color base_color = { 0u };
    f32 color_intensity = 0.0f;

    base_color = (Color) { 0x21, 0x36, 0x8F, 0xFF, };
    color_intensity = (f32) cell->altitude / (f32) ALTITUDE_MIN;

    if (cell->altitude > 0) {
        base_color = (Color) { 0x90, 0x5F, 0x07, 0xFF, };
        color_intensity = (f32) cell->altitude / (f32) ALTITUDE_MAX;
    }
    base_color.a = 64u + (u8) ((color_intensity) * (f32) (255u-64u));

    DrawPoly(*((Vector2*) &target_shape->center), HEXAGON_SIDES_NB, target_shape->radius, 0.0f, base_color);
}

// -------------------------------------------------------------------------------------------------
static void altitude_seed(hexaworld_t *world) {
    hexa_cell_t *tmp_cell = NULL;

    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            tmp_cell = world->tiles[x] + y;

            if (hexa_cell_has_flag(tmp_cell, HEXAW_FLAG_MOUNTAIN)) {
                tmp_cell->altitude = ALTITUDE_MAX;
            } else if (hexa_cell_has_flag(tmp_cell, HEXAW_FLAG_UNDERWATER_CANYONS)) {
                tmp_cell->altitude = ALTITUDE_MIN;
            } else if (tmp_cell->altitude > 0) {
                tmp_cell->altitude = ALTITUDE_MAX / 4;
            } else {
                tmp_cell->altitude = ALTITUDE_MIN / 4;
            }
        }
    }

}

// -------------------------------------------------------------------------------------------------
static void altitude_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    i32 maximum_altitude = (i32) 0x80000000;
    i32 minimum_altitude = (i32) 0x7FFFFFFF;

    hexa_cell_t *tmp_cell = NULL;

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = ((hexa_cell_t *) neighbors[i]);

        if (tmp_cell->altitude < minimum_altitude) {
            minimum_altitude = tmp_cell->altitude;
        }
        if (tmp_cell->altitude > maximum_altitude) {
            maximum_altitude = tmp_cell->altitude;
        }
    }

    if ((cell->altitude > minimum_altitude) && (cell->altitude < maximum_altitude)) {
        cell->altitude = (maximum_altitude + minimum_altitude) / 2;
    }
}

// -------------------------------------------------------------------------------------------------
// -- WINDS -------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void winds_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    vector_2d_cartesian_t translated_vec = { 0u };

    DrawPoly(*(
            (Vector2*) &target_shape->center), 
            HEXAGON_SIDES_NB, 
            target_shape->radius, 
            0.0f, 
            (Color) { 200u, 200u, 200u, (u8) (((f32) (cell->altitude * (cell->altitude > 0)) / (f32) ALTITUDE_MAX) * 255u )}
    );

    DrawCircle(target_shape->center.v, target_shape->center.w, (target_shape->radius/2)*(1.0f-cell->winds_vector.magnitude), (Color) { 0x6e, 0xBA, 0xFF, 0xFF });

    translated_vec = vector2d_polar_to_cartesian(cell->winds_vector);
    DrawLineV(
            *((Vector2*) (&target_shape->center)), 
            (Vector2) { 
                    .x = target_shape->center.v + translated_vec.v * target_shape->radius, 
                    .y = target_shape->center.w + translated_vec.w * target_shape->radius },
            DARKBLUE
    );
}

// -------------------------------------------------------------------------------------------------
static void winds_seed(hexaworld_t *world) {
    // reap the storm ?
    f32 starting_angle = ((f32) (rand() % WINDS_VECTOR_DIRECTIONS_NB)) * (WINDS_VECTOR_UNIT_ANGLE);
    
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].winds_vector = (vector_2d_polar_t) {
                    .angle = starting_angle,
                    .magnitude = 1.0f
            };
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void winds_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *tmp_cell = NULL;
    f32 x_axis_sum = 0.0f;
    f32 y_axis_sum = 0.0f;
    f32 mean_weight = 0.0f;
    f32 mean_angle = 0.0f;
    f32 normalized_altitude_diff = 0.0f;

    // since the angle is not always divisible by 6, the wind goes to two cells
    size_t possible_directions[2u] = { 0u };
    i32 cell_altitudes[2u] = { 0 };
    // cell index later chosen as the least resistance
    size_t definitive_direction = 0u;

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        tmp_cell = ((hexa_cell_t *) neighbors[i]);
        x_axis_sum += cos(tmp_cell->winds_vector.angle);
        y_axis_sum += sin(tmp_cell->winds_vector.angle);
    }

    mean_weight = 1.0f / (f32) DIRECTIONS_NB;
    mean_angle = atan2f(mean_weight * x_axis_sum, mean_weight * y_axis_sum);
    cell->winds_vector.angle = mean_angle;

    // leftmost cell
    possible_directions[0u] = (size_t) ceilf((cell->winds_vector.angle / (PI_T_2)) * (f32) DIRECTIONS_NB) % DIRECTIONS_NB;
    cell_altitudes[0u] = ((hexa_cell_t *) neighbors[possible_directions[0u]])->altitude;
    cell_altitudes[0u] *= (cell_altitudes[0u] > 0); /* altitude below 0 has water over it */
    // rightmost cell
    possible_directions[1u] = (size_t) floorf((cell->winds_vector.angle / (PI_T_2)) * (f32) DIRECTIONS_NB) % DIRECTIONS_NB;
    cell_altitudes[1u] = ((hexa_cell_t *) neighbors[possible_directions[1u]])->altitude;
    cell_altitudes[1u] *= (cell_altitudes[1u] > 0); /* altitude below 0 has water over it */

    definitive_direction = (cell_altitudes[0u] < cell_altitudes[1u])
            ? possible_directions[0u]
            : possible_directions[1u];
    
    // difference between the two "winded upon" cells
    normalized_altitude_diff = (f32) abs(cell_altitudes[0u] - cell_altitudes[1u]) / (f32) ALTITUDE_MAX;
    cell->winds_vector.angle += ((definitive_direction / (f32) DIRECTIONS_NB) * (PI_T_2)) * normalized_altitude_diff;

    // difference between the current cell's altitude and the main winded upon cell
    normalized_altitude_diff = (f32) abs(cell->altitude * (cell->altitude > 0) - cell_altitudes[definitive_direction]) / (f32) ALTITUDE_MAX;
    cell->winds_vector.magnitude = (1.0f - normalized_altitude_diff);
}

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

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void hexaworld_draw_grid(hexaworld_t *world, f32 rectangle_target[4u]) {
    hexagon_shape_t shape = { 0u };

    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            shape = hexagon_position_in_rectangle(
                    rectangle_target,
                    x, y,
                    world->width, world->height
            );
            DrawPolyLines(*((Vector2*) &shape.center), HEXAGON_SIDES_NB, shape.radius, 0.0f, BLACK);
        }
    }
}
