/**
 * @file windowregion.h
 * @author gabriel 
 * @brief Declares constants, data types and methods relating to subdivising a raylib window between different data / methods
 * @version 0.1
 * @date 2023-04-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __WINDOWREGION_H__
#define __WINDOWREGION_H__

#include <unstandard.h>
#include <raylib.h>

/**
 * @brief function handling a click on the region
 */
typedef void (*on_region_click_func_t)(vector_2d_cartesian_t region_dim, u32 relative_x, u32 relative_y, void *data);
/**
 * @brief function handling the drawing of the region to its internal buffer
 */
typedef void (*on_region_refreshed_func_t)(vector_2d_cartesian_t target_dim, void *data);

/**
 * @brief Opaque type to the data defining a window region
 */
typedef struct window_region_t window_region_t;

/**
 * @brief Creates a region data structure on the heap from some information.
 * The internal texture buffer size will be set to the real size of the region relative to the window.
 * 
 * @param[in] ratio_coords_rectangle ratios of the window's sizes, representing the zone attributed to the region
 * @param[in] window_width window pixel width
 * @param[in] window_height window pixel height
 * @param[in] on_click_f function to be called on a click on the region (can be NULL)
 * @param[in] on_refresh_f function to be called when the region needs to be re-drawn (can be NULL)
 * @param[in] related_data pointer to some anonymous data needed by the two previous functions
 * @return window_region_t * a pointer to the new region, or NULL if allocation failed.
 */
window_region_t * window_region_create(
            const f32 ratio_coords_rectangle[4u],
            u32 window_width,
            u32 window_height,
            on_region_click_func_t on_click_f,
            on_region_refreshed_func_t on_refresh_f,
            void *related_data);
/**
 * @brief Re-draw a region's state to its internal texture.
 * 
 * @param[inout] w_region target region
 */
void window_region_refresh(window_region_t *w_region);

/**
 * @brief Draws the internal texture to the active raylib context.
 * 
 * @param[in] w_region target region
 */
void window_region_draw(window_region_t *w_region);
/**
 * @brief Processes a click at a certain pair of coordinates using the data held in the region structure.
 * 
 * @param[inout] w_region target region
 * @param[in] x mouse click x-coordinate
 * @param[in] y mouse click y-coordinate
 */
void window_region_process_click(window_region_t *w_region, i32 x, i32 y);

/**
 * @brief Releases resources held by the structure, and sets to NULL the given pointer.
 * 
 * @param[inout] w_region target region
 */
void window_region_destroy(window_region_t **w_region);

/**
 * @brief Notifies a region that it has changed.
 * 
 * @param[inout] w_region target window region
 */
void window_region_notify_changed(window_region_t *w_region);

#endif
