/**
 * @file hexaworld_application.h
 * @author gabriel 
 * @brief Manages data about a raylib front for an hexa-tiled world.
 * @version 0.1
 * @date 2023-04-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __HEXAWORLD_APPLICATION_H__
#define __HEXAWORLD_APPLICATION_H__

#include <unstandard.h>

/**
 * @brief Application data as an opaque type.
 */
typedef struct hexaworld_raylib_app_handle_t hexaworld_raylib_app_handle_t;

// /**
//  * @brief Creates the application data on the heap.
//  * 
//  * @param[in] window_width horizontal size, in pixels, of the window
//  * @param[in] window_height vertical size, in pixels, of the window
//  * @param[in] world_width horizontal length of the world, in number of tiles
//  * @param[in] world_height vertical length of the world, in number of tiles
//  * @return hexaworld_raylib_app_handle_t* a pointer to the application data instance, or NULL if it could not be allocated
//  */
// hexaworld_raylib_app_handle_t *hexaworld_raylib_app_create(u32 window_width, u32 window_height, u32 world_width, u32 world_height);

// /**
//  * @brief Releases the application data from the heap.
//  * 
//  * @param[inout] hexapp pointer to a pointer to the application data to free, the pointed pointer is set to NULL
//  */
// void hexaworld_raylib_app_destroy(hexaworld_raylib_app_handle_t **hexapp);

void hexaworld_raylib_app_deinit(hexaworld_raylib_app_handle_t **hexapp);

/**
 * @brief Initialize the application in a "ready" state from which it can be ran.
 * This will start an empty raylib window.
 * 
 * @param[inout] hexapp a non-NULL pointer to some application data
 * @param[in] random_seed seed for the random number generator
 */
hexaworld_raylib_app_handle_t * hexaworld_raylib_app_init(i32 random_seed, u32 window_width, u32 window_height, u32 world_width, u32 world_height);

/**
 * @brief Runs the application until the window is closed. 
 * If the data is uninitialised or some mishaps happened during initialisation, the function exits immediatelly.
 * 
 * @param[inout] hexapp a non-NULL pointer to some application data
 * @param[in] target_fps target window render fps.
 */
void hexaworld_raylib_app_run(hexaworld_raylib_app_handle_t *hexapp, u32 target_fps);

#endif
