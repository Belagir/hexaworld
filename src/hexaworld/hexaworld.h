/**
 * @file hexaworld.h
 * @author gabriel 
 * @brief Defines the interface to manipulate the data representing a hexagonally tiled world.
 * @version 0.1
 * @date 2023-04-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __HEXAWORLD_H__
#define __HEXAWORLD_H__

#include <unstandard.h>

/**
 * @brief List of the world's unique layers.
 */
typedef enum hexaworld_layer_t {
    HEXAW_LAYER_TELLURIC,   ///< Layer representing the movement of the tectonic plates.
    HEXAW_LAYER_LANDMASS,   ///< Layer representing the raw landmasses without elevation.
    HEXAW_LAYER_ALTITUDE,   ///< Layer representing the different altitudes of the world.
    HEXAW_LAYER_WINDS,      ///< Layer representing the mean wind direction and force over the land.

    HEXAW_LAYERS_NUMBER     ///< Total number of layers.
} hexaworld_layer_t;

/**
 * @brief Data representing an hexa-tiled world as an opaque type.
 */
typedef struct hexaworld_t hexaworld_t;

/**
 * @brief Creates an empty, zero-initialized world on the heap.
 * 
 * @param[in] width number of tiles on the x-axis
 * @param[in] height number of tiles on the y-axis
 * @return hexaworld_t* a pointer to the world data, NULL if allocation failed
 */
hexaworld_t *hexaworld_create_empty(size_t width, size_t height);

/**
 * @brief Deallocates the world and sets the pointer to NULL.
 * 
 * @param[inout] world double pointer to some world data.
 */
void hexaworld_destroy(hexaworld_t **world);

/**
 * @brief Draw a layer of the world to the currently active, caller-dependent raylib context. 
 * 
 * @param[in] world non-NULL pointer to some world data
 * @param[in] layer drawn layer
 * @param[in] rectangle_target screen draw space allocated to the world
 */
void hexaworld_draw(hexaworld_t *world, hexaworld_layer_t layer, f32 rectangle_target[4u]);

/**
 * @brief Generates a single layer of the world.
 * 
 * @param[inout] world non-NULL pointer to some world data
 * @param[in] layer (re-)generated layer
 */
void hexaworld_genlayer(hexaworld_t *world, hexaworld_layer_t layer);

/**
 * @brief Sets all the layer's data to a blank state.
 * 
 * @param[inout] world target world.
 */
void hexaworld_raze(hexaworld_t *world);

#endif
