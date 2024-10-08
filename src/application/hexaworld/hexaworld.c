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

#include <math.h>
#include <stdlib.h>
#include <raylib.h>

#include <cellotomaton.h>
#include <colorpalette.h>

#include "worldcomponents/hexaworldcomponents.h"

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DECLARATIONS --------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Draw an empty hexagonal grid on the current raylib drawing context.
 * 
 * @param[in] world non-NULL pointer to some world data
 * @param[in] rectangle_target rectangle defined by the topleft coordinates and its sides' length (in pixels)
 */
static void hexaworld_draw_grid(hexaworld_t *world, f32 rectangle_target[4u]);

// -------------------------------------------------------------------------------------------------
// ---- HEADER FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
hexaworld_t *hexaworld_create_empty(size_t width, size_t height, i32 random_seed) {
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

    // adding the tectonic plates layer
    world->hexaworld_layers_functions[HEXAW_LAYER_TELLURIC]    = telluric_layer_calls;
    // adding the rough landmass layer
    world->hexaworld_layers_functions[HEXAW_LAYER_LANDMASS]    = landmass_layer_calls;
    // adding the altitude gradient layer
    world->hexaworld_layers_functions[HEXAW_LAYER_ALTITUDE]    = altitude_layer_calls;
    // adding the mean temperature layer
    world->hexaworld_layers_functions[HEXAW_LAYER_TEMPERATURE] = temperature_layer_calls;
    // adding the wind mean direction layer
    world->hexaworld_layers_functions[HEXAW_LAYER_WINDS]       = winds_layer_calls;
    // adding the cloud cover layer
    world->hexaworld_layers_functions[HEXAW_LAYER_CLOUD_COVER] = cloud_cover_layer_calls;
    // adding the river & lakes layer
    world->hexaworld_layers_functions[HEXAW_LAYER_FRESHWATER]  = freshwater_layer_calls;
    // adding the vegetation cover layer
    world->hexaworld_layers_functions[HEXAW_LAYER_VEGETATION]  = vegetation_layer_calls;
    // adding the nice overworld drawing layer
    world->hexaworld_layers_functions[HEXAW_LAYER_WHOLE_WORLD] = whole_world_layer_calls;

    world->width = width;
    world->height = height;
    world->map_seed = random_seed;

    return world;
}

// -------------------------------------------------------------------------------------------------
void hexaworld_destroy(hexaworld_t **world) {

    if (*world) {
        if ((*world)->tiles) {
            for (size_t i = 0u ; i < (*world)->width; i++) {
                if ((*world)->tiles[i]) {
                    free((*world)->tiles[i]);
                }
            }
            free((*world)->tiles);
        }

        otomaton_destroy(&((*world)->automaton));

        (*world)->width = 0u;
        (*world)->height = 0u;

        free((*world));
    }

    *world = NULL;
}

// -------------------------------------------------------------------------------------------------
void hexaworld_draw(hexaworld_t *world, hexaworld_layer_t layer, f32 rectangle_target[4u]) {
    layer_draw_function_t layer_function = NULL;
    hexagon_shape_t shape = { 0u };

    layer_function = world->hexaworld_layers_functions[layer].draw_func;

    srand(world->map_seed ^ layer);

    // drawing each cell
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            shape = hexagon_pixel_position_in_rectangle(rectangle_target, x, y, world->width, world->height);
            draw_hexagon(&shape, COLOR_WHITE, 1.0f, DRAW_HEXAGON_FILL);
            layer_function(world->tiles[x] + y, &shape);
        }
    }

    hexaworld_draw_grid(world, rectangle_target);
}

// -------------------------------------------------------------------------------------------------
void hexaworld_genlayer(hexaworld_t *world, hexaworld_layer_t layer) {
    size_t iteration_number = 0u;

    srand(world->map_seed ^ layer);

    if (world->hexaworld_layers_functions[layer].seed_func) {
        world->hexaworld_layers_functions[layer].seed_func(world);
    }

    iteration_number = world->hexaworld_layers_functions[layer].automaton_iter;
    if (world->hexaworld_layers_functions[layer].iteration_flavour == LAYER_GEN_ITERATE_RELATIVE) {
        iteration_number *= (u32) sqrt(powf((f32) world->width, 2.0f) + powf((f32) world->height, 2.0f)) / 10;
    }

    // applying the overall generation function N times
    otomaton_apply(world->automaton, iteration_number, world->hexaworld_layers_functions[layer].automaton_func);

    // if the flag gneration function exists, apply it one time
    if (world->hexaworld_layers_functions[layer].flag_gen_func) {
        otomaton_apply(world->automaton, 1u, world->hexaworld_layers_functions[layer].flag_gen_func);
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
void hexaworld_reseed(hexaworld_t *world, i32 new_seed) {
    world->map_seed = new_seed;
}

// -------------------------------------------------------------------------------------------------
hexa_cell_t *hexaworld_tile_at(hexaworld_t *world, u32 x, u32 y, f32 reference_rectangle[4u], u32 *out_x, u32 *out_y) {
    vector_2d_cartesian_t array_coords = { 0u };
    i32 wanted_x = 0;
    i32 wanted_y = 0;

    array_coords = hexagon_array_coords_from_rectangle(reference_rectangle, x, y, world->width, world->height);

    wanted_x = (i32) floor(array_coords.v);
    wanted_y = (i32) floor(array_coords.w);
    
    if ((wanted_x < 0) || (wanted_x >= (i32) world->width) || (wanted_y < 0) || (wanted_y >= (i32) world->height)) {
        *out_x = world->width;
        *out_y = world->height;
        
        return NULL;
    }

    *out_x = wanted_x;
    *out_y = wanted_y;

    return world->tiles[wanted_x] + wanted_y;
}

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void hexaworld_draw_grid(hexaworld_t *world, f32 rectangle_target[4u]) {
    hexagon_shape_t shape = { 0u };

    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            shape = hexagon_pixel_position_in_rectangle(
                    rectangle_target,
                    x, y,
                    world->width, world->height
            );
            draw_hexagon(&shape, COLOR_BLACK, 1.0f, DRAW_HEXAGON_LINES);
        }
    }
}
