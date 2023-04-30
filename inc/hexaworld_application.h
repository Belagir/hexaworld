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
 * @brief Application data handle as an opaque type.
 */
typedef struct hexaworld_raylib_app_handle_t hexaworld_raylib_app_handle_t;

/**
 * @brief De-initialize the data beihnd an application handle and sets the handle to NULL
 * 
 * @param hexapp double pointer to an initialized application handle
 */
void hexaworld_raylib_app_deinit(hexaworld_raylib_app_handle_t **hexapp);

/**
 * @brief Initialize the application in a "ready" state from which it can be ran and returns a handle to the data.
 * This will start an empty raylib window. If this function is ran multiple times, all the returned handles point
 * to the same data.
 * 
 * @param random_seed any intgerer that will be used to seed the random number generator.
 * @param window_width width of the raylib window, in pixels
 * @param window_height height of the raylib window, in pixels
 * @param world_width width of the world, in number of tiles
 * @param world_height height of the world, in number of tiles
 * @return hexaworld_raylib_app_handle_t* a handle to the application service data
 */
hexaworld_raylib_app_handle_t * hexaworld_raylib_app_init(i32 random_seed, u32 window_width, u32 window_height, u32 world_width, u32 world_height);

/**
 * @brief Runs the application until the window is closed. 
 * If the data is uninitialised or some mishaps happened during initialisation, the function exits immediately.
 * 
 * @param[inout] hexapp a non-NULL pointer to some application data
 * @param[in] target_fps target window render fps.
 */
void hexaworld_raylib_app_run(hexaworld_raylib_app_handle_t *hexapp, u32 target_fps);

#endif
