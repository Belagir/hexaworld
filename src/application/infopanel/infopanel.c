/**
 * @file infopanel.c
 * @author gabriel 
 * @brief DEfinition file for the info panel object.
 * @version 0.1
 * @date 2023-05-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "infopanel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <colorpalette.h>
#include <raylib.h>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

#define TILE_FLAG_NO_DESCRIPTION ("")     ///< constant to signal that a cell flag has no description 

#define TILE_INFO_BUFFER_SIZE (1024u)       ///< number of ascii signs that an info panel can display
#define TILE_INFO_FORMAT_STRING ("TILE AT %3d : %3d\n - mean altitude : % 6dm\n - mean temperature : %+ 3d°C\n - mean cloud cover : %.1f%%\n- vegetation cover : %.1f%%\n - vegetation trees : %.1f%%\n\n")        ///< main format string do display cell information
#define TILE_INFO_FONT_SIZE (18)        ///< font size for the panel

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
typedef struct info_panel_t {
    /// currently examined cell, watch out for NULL !
    hexa_cell_t *target_cell;
    /// cell's x position
    u32 cell_x;
    /// cell's y position
    u32 cell_y;

    /// pointer to the string buffer
    char *tile_description_buffer;
} info_panel_t;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

const char *tile_flag_description_map[HEXAW_FLAGS_NB] = {
        // HEXAW_FLAG_TELLURIC_RIDGE
        TILE_FLAG_NO_DESCRIPTION,
        // HEXAW_FLAG_TELLURIC_RIFT
        TILE_FLAG_NO_DESCRIPTION,
        // HEXAW_FLAG_MOUNTAIN
        "MOUNTAINS\n",
        // HEXAW_FLAG_ISLES
        "ISLES\n",
        // HEXAW_FLAG_CANYONS
        "CANYONS\n",
        // HEXAW_FLAG_UNDERWATER_CANYONS
        "ABYSS\n",
        // HEXAW_FLAG_SMALL_COAST,
        "SMALL COASTLINE\n",
        // HEXAW_FLAG_LONG_COAST,
        "LONG COASTLINE\n",
        // HEXAW_FLAG_MEANDERS
        "WETLANDS\n",
        // HEXAW_FLAG_WATERFALLS
        "WATERFALLS\n",
        // HEXAW_FLAG_RIVER_MOUTH
        TILE_FLAG_NO_DESCRIPTION,
        // HEXAW_FLAG_LAKE
        "LAKE\n"
};

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Updates the description buffer from the examined cell
 * 
 * @param[inout] panel target panel
 */
void update_tile_description_buffer(info_panel_t *panel);

// -------------------------------------------------------------------------------------------------
info_panel_t *info_panel_create(void) {
    info_panel_t *panel = NULL;

    panel = malloc(sizeof(*panel));
    if (!panel) {
        return NULL;
    }

    panel->tile_description_buffer = malloc(sizeof(*panel->tile_description_buffer) * TILE_INFO_BUFFER_SIZE);
    panel->target_cell = NULL;
    panel->cell_x = 0u;
    panel->cell_y = 0u;

    update_tile_description_buffer(panel);

    return panel;
}

// -------------------------------------------------------------------------------------------------
void info_panel_destroy(info_panel_t **panel) {
    if ((!panel) || (!(*panel))) {
        return;
    }

    if ((*panel)->tile_description_buffer) {
        free((*panel)->tile_description_buffer);
        (*panel)->tile_description_buffer = NULL;
    }

    (*panel)->target_cell = NULL;
    free(*panel);

    (*panel) = NULL;
}

// -------------------------------------------------------------------------------------------------
void info_panel_set_examined_cell(info_panel_t *panel, hexa_cell_t *cell, u32 cell_pos_x, u32 cell_pos_y) {
    if (!panel) {
        return;
    }

    panel->target_cell = cell;
    panel->cell_x = cell_pos_x;
    panel->cell_y = cell_pos_y;

    update_tile_description_buffer(panel);
}

// -------------------------------------------------------------------------------------------------
void info_panel_draw(info_panel_t *panel) {
    ClearBackground(AS_RAYLIB_COLOR(COLOR_WHITE));

    DrawText(panel->tile_description_buffer, 5, 5, TILE_INFO_FONT_SIZE, AS_RAYLIB_COLOR(COLOR_BLACK));
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------
void update_tile_description_buffer(info_panel_t *panel) {
    if (!panel->target_cell){
        panel->tile_description_buffer[0] = '\0';
        return;
    }

    snprintf(panel->tile_description_buffer, TILE_INFO_BUFFER_SIZE, TILE_INFO_FORMAT_STRING,
            panel->cell_x,
            panel->cell_y,
            panel->target_cell->altitude,
            panel->target_cell->temperature,
            panel->target_cell->cloud_cover * 100.0f,
            panel->target_cell->vegetation_cover * 100.0f,
            panel->target_cell->vegetation_trees * 100.0f
    );

    for (size_t i_flag = 0u ; i_flag < HEXAW_FLAGS_NB ; i_flag++) {
        if (hexa_cell_has_flag(panel->target_cell, i_flag)) {
            strncat(panel->tile_description_buffer, tile_flag_description_map[i_flag], strlen(tile_flag_description_map[i_flag]));
        }
    }
}
