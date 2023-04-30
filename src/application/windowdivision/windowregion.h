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
 * @brief Lists the registered window region in the application
 */
typedef enum registered_window_region_t {
    WINREGION_HEXAWORLD,
    WINREGION_TILEINFO,

    WINREGIONS_NUMBER
} registered_window_region_t;

/**
 * @brief function handling a click on the region
 */
typedef void (*on_region_click_func_t)(vector_2d_cartesian_t region_dim, u32 relative_x, u32 relative_y, void *data);
/**
 * @brief function handling the drawing of the region to its internal buffer
 */
typedef void (*on_region_refreshed_func_t)(vector_2d_cartesian_t target_dim, void *data);

/**
 * @brief Data defining a window region
 */
typedef struct window_region_t {
    /// actual window-relative pixel coordinates of the region
    f32 px_coords_rectangle[4u];
    
    /// function executed when the user clicks on the region 
    on_region_click_func_t on_click_f;
    /// function executed when something needs to refresh the buffer from the internal data
    on_region_refreshed_func_t on_refresh_f;
    /// pointer to some data related to the region's functionality
    void *related_data;

    /// texture buffer for rendering the region 
    RenderTexture2D buffer_rendertexture;
} window_region_t;

/**
 * @brief Initialize a region data from some information.
 * The internal texture buffer size will be set to the real size of the region relative to the window.
 * 
 * @param w_region target window region data
 * @param ratio_coords_rectangle ratios of the window's sizes, representing the zone attributed to the region
 * @param window_width window pixel width
 * @param window_height window pixel height
 * @param on_click_f function to be called on a click on the region (can be NULL)
 * @param on_refresh_f function to be called when the region needs to be re-drawn (can be NULL)
 * @param related_data pointer to some anonymous data needed by the two previous functions
 */
void window_region_init(
            window_region_t *w_region,
            const f32 ratio_coords_rectangle[4u],
            u32 window_width,
            u32 window_height,
            on_region_click_func_t on_click_f,
            on_region_refreshed_func_t on_refresh_f,
            void *related_data);
/**
 * @brief Re-draw a region's state to its internal texture.
 * 
 * @param w_region target region
 */
void window_region_refresh(window_region_t *w_region);

/**
 * @brief Draws the internal texture to the active raylib context.
 * 
 * @param w_region target region
 */
void window_region_draw(window_region_t *w_region);
/**
 * @brief Processes a click at a certain pair of coordinates using the data held in the region structure.
 * 
 * @param w_region target region
 * @param x mouse click x-coordinate
 * @param y mouse click y-coordinate
 */
void window_region_process_click(window_region_t *w_region, i32 x, i32 y);

/**
 * @brief Releases resources held by the structure.
 * 
 * @param w_region target region
 */
void window_region_deinit(window_region_t *w_region);


#endif
