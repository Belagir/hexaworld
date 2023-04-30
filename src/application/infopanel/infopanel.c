
#include "infopanel.h"

#include <stdlib.h>

typedef struct info_panel_t {
    hexa_cell_t *examined_cell;
} info_panel_t;

info_panel_t *info_panel_create(void) {
    info_panel_t *panel = NULL;

    panel = malloc(sizeof(*panel));
    if (!panel) {
        return NULL;
    }

    panel->examined_cell = NULL;

    return panel;
}

void info_panel_destroy(info_panel_t **panel) {
    if (!(*panel)) {
        return;
    }

    free(*panel);
    *panel = NULL;
}
