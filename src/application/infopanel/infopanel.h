
#ifndef __INFOPANEL_H__
#define __INFOPANEL_H__

#include <unstandard.h>
#include <hexatypes.h>

typedef struct info_panel_t info_panel_t;

info_panel_t *info_panel_create(void);

void info_panel_destroy(info_panel_t **panel);

#endif
