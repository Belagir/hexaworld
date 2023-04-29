
#include "hexaworldcomponents.h"

// -------------------------------------------------------------------------------------------------
void hexa_cell_set_flag(hexa_cell_t *cell, u32 flag) {
    cell->flags = (cell->flags | (0x01 << flag));
}

// -------------------------------------------------------------------------------------------------
u32 hexa_cell_has_flag(hexa_cell_t *cell, u32 flag) {
    return (cell->flags & (0x01 << flag));
}
