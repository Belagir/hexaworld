/**
 * @file infopanel.h
 * @author gabriel 
 * @brief Declaration file for the info panel utility
 * @version 0.1
 * @date 2023-05-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __INFOPANEL_H__
#define __INFOPANEL_H__

#include <unstandard.h>
#include <hexagonparadigm.h>

/**
 * @brief Opaque type to manipulate the info panel data.
 */
typedef struct info_panel_t info_panel_t;

/**
 * @brief Allocates the info panel on the heap.
 * 
 * @return info_panel_t* pointer to the new info panel data, or NULL if the allocation failed.
 */
info_panel_t *info_panel_create(void);

/**
 * @brief Releases the resources held by an info panel and sets the pointer to null.
 * 
 * @param[inout] panel double pointer to some info panel data
 */
void info_panel_destroy(info_panel_t **panel);

/**
 * @brief Sets the currently lokked cell pointer in an info panel structure to a new value.
 * 
 * @param[inout] panel target panel
 * @param[in] cell new cell pointer (can be NULL for "no cell")
 * @param[in] cell_pos_x new cell's x-coordinate
 * @param[in] cell_pos_y new cell's y-coordinate
 */
void info_panel_set_examined_cell(info_panel_t *panel, hexa_cell_t *cell, u32 cell_pos_x, u32 cell_pos_y);

/**
 * @brief Sets the map seed displayed by the infopanel.
 * 
 * @param[inout] panel target panel
 * @param[in] map_seed current seed of the map
 */
void info_panel_set_map_seed(info_panel_t *panel, i32 map_seed);

/**
 * @brief Draws the info panel to the current raylib context.
 * 
 * @param[in] panel target panel 
 */
void info_panel_draw(info_panel_t *panel);

#endif
