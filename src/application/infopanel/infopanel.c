
#include "infopanel.h"

#include <stdlib.h>

#include <colorpalette.h>
#include <raylib.h>

// -------------------------------------------------------------------------------------------------
typedef struct info_panel_t {
    hexa_cell_t *target_cell;
} info_panel_t;

// -------------------------------------------------------------------------------------------------
info_panel_t *info_panel_create(void) {
    info_panel_t *panel = NULL;

    panel = malloc(sizeof(*panel));

    return panel;
}

// -------------------------------------------------------------------------------------------------
void info_panel_destroy(info_panel_t **panel) {
    if ((!panel) || (!(*panel))) {
        return;
    }

    (*panel)->target_cell = NULL;
    free(*panel);

    (*panel) = NULL;
}

// -------------------------------------------------------------------------------------------------
void info_panel_set_examined_cell(info_panel_t *panel, hexa_cell_t *cell) {
    if (!panel) {
        return;
    }

    panel->target_cell = cell;
}

// -------------------------------------------------------------------------------------------------
void info_panel_draw(info_panel_t *panel) {
    ClearBackground(AS_RAYLIB_COLOR(COLOR_WHITE));
}
