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

    world->hexaworld_layers_functions[HEXAW_LAYER_TELLURIC]   = telluric_layer_calls;
    world->hexaworld_layers_functions[HEXAW_LAYER_LANDMASS]   = landmass_layer_calls;
    world->hexaworld_layers_functions[HEXAW_LAYER_ALTITUDE]   = altitude_layer_calls;
    world->hexaworld_layers_functions[HEXAW_LAYER_WINDS]      = winds_layer_calls;
    world->hexaworld_layers_functions[HEXAW_LAYER_HUMIDITY]   = humidity_layer_calls;
    world->hexaworld_layers_functions[HEXAW_LAYER_FRESHWATER] = freshwater_layer_calls;

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

    layer_function = world->hexaworld_layers_functions[layer].draw_func;

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

    world->hexaworld_layers_functions[layer].seed_func(world);

    iteration_number = world->hexaworld_layers_functions[layer].automaton_iter;
    if (world->hexaworld_layers_functions[layer].iteration_flavour == LAYER_GEN_ITERATE_RELATIVE) {
        iteration_number *= (world->width / 10);
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
// ---- STATIC FUNCTIONS DEFINITIONS ---------------------------------------------------------------
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
