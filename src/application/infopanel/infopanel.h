
#ifndef __INFOPANEL_H__
#define __INFOPANEL_H__

#include <unstandard.h>
#include <hexatypes.h>

/**
 * @brief 
 * 
 */
typedef struct info_panel_t info_panel_t;

/**
 * @brief 
 * 
 * @return info_panel_t* 
 */
info_panel_t *info_panel_create(void);

/**
 * @brief 
 * 
 * @param panel 
 */
void info_panel_destroy(info_panel_t **panel);

/**
 * @brief 
 * 
 * @param panel 
 * @param cell 
 */
void info_panel_set_examined_cell(info_panel_t *panel, hexa_cell_t *cell);

/**
 * @brief 
 * 
 * @param panel 
 */
void info_panel_draw(info_panel_t *panel);

#endif
