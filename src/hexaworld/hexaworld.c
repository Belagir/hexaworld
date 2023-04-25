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

#define HEXAGON_SIDES_NB (6u)     ///< number of sides of an hexagon. tough.

#define SQRT_OF_3 (1.73205f)      ///< approximation of the square root of 3
#define THREE_HALVES (1.5f)       ///< not an *approximation* of 3 / 2

#define ITERATION_NB_TELLURIC (2u)    ///< number of automaton iteration for the telluric layer
#define ITERATION_NB_LANDMASS (6u)    ///< number of automaton iteration for the landmass layer
#define ITERATION_NB_ALTITUDE (10u)    ///< number of automaton iteration for the altitude layer

#define TELLURIC_VECTOR_SEEDING_INV_CHANCE (0x40)  ///< the greater, the bigger the chance a telluric tile is NOT seeded.
#define TELLURIC_VECTOR_DIRECTIONS_NB (6)     ///< number of possible directions for a telluric vector
#define TELLURIC_VECTOR_UNIT_ANGLE (((2.0f) * (PI)) / (TELLURIC_VECTOR_DIRECTIONS_NB))      ///< telluric vector minimum angle 

#define LANDMASS_SEEDING_CHANCE (0x03)    ///< the greater, the bigger the chance a land tile is seeded.

#define ALTITUDE_MAX (3000)  ///< maximum altitude, in meters
#define ALTITUDE_MIN (-2000)  ///< minimum altitude, in meters

// -------------------------------------------------------------------------------------------------
// ---- TYPE DEFINITIONS ---------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Describes wether a cell automaton should iterates an absolute number of times or a number of times relative to the array size.
 */
typedef enum layer_gen_iteration_type_t {
    LAYER_GEN_ITERATE_ABSOLUTE,
    LAYER_GEN_ITERATE_RELATIVE,
} layer_gen_iteration_type_t;

/**
 * @brief Possible flags held in a cell. (They claim they're innocent)
 */
typedef enum hexaworld_cell_flag_t {
    HEXAW_FLAG_TELLURIC_RIDGE,      ///< Two plates are clashing here, and forming mountains
    HEXAW_FLAG_TELLURIC_RIFT,       ///< Two plates are growing from the ocean's floor, forming a deep crevasse

    HEXAW_FLAG_MOUNTAIN,            ///< Some mountains are forming here, above the water
    HEXAW_FLAG_ISLES,               ///< Some isles have been created by a tectonic force
    HEXAW_FLAG_CANYONS,             ///< Some old tectonic event or river dug a long path here
    HEXAW_FLAG_UNDERWATER_CANYONS,  ///< Tectonic forces are at work here and create some heavy drop on the ocean floor

    HEXAW_FLAGS_NB,     ///< Total number of flags
} hexaworld_cell_flag_t;

/**
 * @brief A single hexagonal cell.
 */
typedef struct hexa_cell_t {
    /// long unsigned integer containing the flags as bit offsets
    u64 flags;
    
    /// local angle of the tectonic plate 
    vector_2d_polar_t telluric_vector;
    /// mean altitude of the tile
    i32 altitude;
} hexa_cell_t;

/**
 * @brief just the shape of an hexagon.
 */
typedef struct hexagon_shape_t {
    vector_2d_cartesian_t center;
    f32 radius;
} hexagon_shape_t;

// -------------------------------------------------------------------------------------------------
typedef struct hexaworld_t { 
    /// 2d heap-allocated array of the tiles
    hexa_cell_t **tiles;
    /// number of tiles on the x-axis
    size_t width;
    /// number of tiles on the y-axis
    size_t height;

    /// pointer to an heap-allocated cellular automaton for layer generation
    cell_automaton_t *automaton;
} hexaworld_t;

/**
 * @brief Function pointer as the prototype of some code handling the drawing of a single cell.
 */
typedef void (*layer_draw_function_t)(hexa_cell_t *cell, hexagon_shape_t *target_shape);

/**
 * @brief Function pointer as the prototype of some code handling the seeding of some layer.
 */
typedef void (*layer_seed_function_t)(hexaworld_t *world);


/**
 * @brief Aggregation of all the functions working on a single layer to create it and display it.
 */
typedef struct layer_calls_t {
    /// function drawing a layer's single cell
    layer_draw_function_t draw_func;
    /// function seeding the whole world before the automaton
    layer_seed_function_t seed_func;
    /// function applied by the automaton to generate a single cell
    apply_to_cell_func_t automaton_func;
    /// function applied by the automaton to create the flags of a single cell
    apply_to_cell_func_t flag_gen_func;
    /// number of times the automaton applies the `automaton_func` toeach cell of the world
    u32 automaton_iter;
    /// way the automaton should iterate over the array
    layer_gen_iteration_type_t iteration_flavour;
} layer_calls_t;

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DECLARATIONS --------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// -- TELLURIC -------------------------------------------------------------------------------------

static void telluric_vector_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void telluric_vector_seed(hexaworld_t *world);

static void telluric_vector_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

static void telluric_vector_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]);

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
// -------------------------------------------------------------------------------------------------

/**
 * @brief Sets an bit flag in a cell.
 * 
 * @param[inout] cell target cell
 * @param[in] flag flag to set to 1
 */
static void hexa_cell_set_flag(hexa_cell_t *cell, u32 flag);

/**
 * @brief Checks for the presence of a bit flag in  a cell.
 * 
 * @param[inout] cell target cell
 * @param[in] flag flag to check
 * @return 1 if the flag is set to 1, 0 otherwise
 */
static u32 hexa_cell_has_flag(hexa_cell_t *cell, u32 flag);

/**
 * @brief Computes the position and radius of a hexagonal cell in a pixel rectangle.
 * 
 * @param[in] boundaries rectangle defined by the topleft coordinates and its sides' length (in pixels)
 * @param[in] x tile x-position in the world array
 * @param[in] y tile y-position in the world array
 * @param[in] width width, in tiles, of the world array
 * @param[in] height height, in tiles, of the world array
 * @return hexagon_shape_t pixel shape of the hexagon representing the cell's coordinates
 */
static hexagon_shape_t hexagon_position_in_rectangle(f32 boundaries[4u], u32 x, u32 y, u32 width, u32 height);

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
    { &telluric_vector_draw, &telluric_vector_seed, &telluric_vector_apply, &telluric_vector_flag_gen, ITERATION_NB_TELLURIC, LAYER_GEN_ITERATE_RELATIVE },
    /// landmass layer calls
    { &landmass_draw,        &landmass_seed,        &landmass_apply,        &landmass_flag_gen,        ITERATION_NB_LANDMASS, LAYER_GEN_ITERATE_ABSOLUTE },
    /// altitude layer calls
    { &altitude_draw,        &altitude_seed,        &altitude_apply,        NULL,                      ITERATION_NB_ALTITUDE, LAYER_GEN_ITERATE_RELATIVE },
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
    world->automaton = otomaton_create(NULL);
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
    otomaton_set_apply_function(world->automaton, hexaworld_layers_functions[layer].automaton_func);
    otomaton_apply(
            world->automaton, 
            iteration_number, 
            (void **) world->tiles, 
            world->width, 
            world->height, 
            sizeof(**world->tiles)
    );

    // if the flag gneration function exists, apply it one time
    if (hexaworld_layers_functions[layer].flag_gen_func) {
        otomaton_set_apply_function(world->automaton, hexaworld_layers_functions[layer].flag_gen_func);
        otomaton_apply(
                world->automaton, 
                1u, 
                (void **) world->tiles, 
                world->width, 
                world->height, 
                sizeof(**world->tiles)
        );
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
static void telluric_vector_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
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
static void telluric_vector_seed(hexaworld_t *world) {
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
static void telluric_vector_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
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
static void telluric_vector_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    hexa_cell_t *pushed_against_cell = NULL;
    size_t pushed_against_cell_index = 0u;

    hexa_cell_t *pushed_from_cell = NULL;
    size_t pushed_from_cell_index = 0u;

    pushed_against_cell_index = (size_t) (((cell->telluric_vector.angle / (2*PI))) * DIRECTIONS_NB);
    pushed_against_cell = neighbors[pushed_against_cell_index];

    pushed_from_cell_index = (size_t) (((fmod(cell->telluric_vector.angle+PI, (2*PI)) / (2*PI))) * DIRECTIONS_NB);
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
    Color tile_color = BLUE;

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_UNDERWATER_CANYONS)) {
        tile_color = DARKBLUE;
    } else if (hexa_cell_has_flag(cell, HEXAW_FLAG_MOUNTAIN)) {
        tile_color = DARKGREEN;
    } else if (hexa_cell_has_flag(cell, HEXAW_FLAG_CANYONS)) {
        tile_color = LIME;
    } else if (cell->altitude > 0) {
        tile_color = GREEN;
    }


    DrawPoly(*((Vector2*) &target_shape->center), HEXAGON_SIDES_NB, target_shape->radius, 0.0f, tile_color);

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_ISLES)) {
        DrawCircleV(*((Vector2*) &target_shape->center), target_shape->radius/2, GREEN);   
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

    base_color = DARKBLUE;
    color_intensity = (f32) cell->altitude / (f32) ALTITUDE_MIN;

    if (cell->altitude > 0) {
        base_color = DARKGREEN;
        color_intensity = (f32) cell->altitude / (f32) ALTITUDE_MAX;
    }
    base_color.a = 32u + (u8) ((color_intensity) * (f32) (225u-32u));

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
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void hexa_cell_set_flag(hexa_cell_t *cell, u32 flag) {
    cell->flags = (cell->flags | (0x01 << flag));
}

// -------------------------------------------------------------------------------------------------
static u32 hexa_cell_has_flag(hexa_cell_t *cell, u32 flag) {
    return (cell->flags & (0x01 << flag));
}

// -------------------------------------------------------------------------------------------------
static hexagon_shape_t hexagon_position_in_rectangle(f32 boundaries[4u], u32 x, u32 y, u32 width, u32 height) {
    hexagon_shape_t shape = { 0u };

    shape.radius = (boundaries[3u] / ((f32) height*THREE_HALVES));

    shape.center = (vector_2d_cartesian_t) { 
            boundaries[0u] + ((((f32) x+0.5f) + (0.5f * (f32) (y & 0x01))) * SQRT_OF_3 * (shape.radius)), 
            boundaries[1u] + ((((f32) y+0.5f)) * THREE_HALVES * (shape.radius)) 
    };

    return shape;
}

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
