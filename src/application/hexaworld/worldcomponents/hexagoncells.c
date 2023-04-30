/**
 * @file hexagoncells.c
 * @author gabriel 
 * @brief Definition file for some cell data interaction functions
 * @version 0.1
 * @date 2023-04-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "hexaworldcomponents.h"

// -------------------------------------------------------------------------------------------------
void hexa_cell_set_flag(hexa_cell_t *cell, u32 flag) {
    cell->flags = (cell->flags | (0x01 << flag));
}

// -------------------------------------------------------------------------------------------------
u32 hexa_cell_has_flag(hexa_cell_t *cell, u32 flag) {
    return (cell->flags & (0x01 << flag));
}
