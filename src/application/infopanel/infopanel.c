
#include "infopanel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <colorpalette.h>
#include <raylib.h>

#define TILE_INFO_BUFFER_SIZE (1024u)

#define TILE_INFO_FORMAT_STRING ("TILE AT %3d : %3d\n - mean altitude : % 6dm\n - mean cloud cover : ~%.1f%%\n - mean temperature : %+ 3d°C\n- vegetation cover : %.1f%%\n")

#define TILE_INFO_FONT_SIZE (18)

// -------------------------------------------------------------------------------------------------
typedef struct info_panel_t {
    hexa_cell_t *target_cell;
    u32 cell_x;
    u32 cell_y;

    char *tile_description_buffer;
} info_panel_t;

/**
 * @brief 
 * 
 * @param panel 
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
            panel->target_cell->humidity * 100.0f,
            panel->target_cell->temperature,
            panel->target_cell->vegetation_cover * 100.0f
    );
}
