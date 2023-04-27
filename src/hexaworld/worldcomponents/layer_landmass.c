
#include "hexaworldcomponents.h"

#include <stdlib.h>

#include <raylib.h>

#define ITERATION_NB_LANDMASS (6u)    ///< number of automaton iteration for the landmass layer

static void landmass_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape);

static void landmass_seed(hexaworld_t *world);

static void landmass_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]);

static void landmass_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]);


// -------------------------------------------------------------------------------------------------
// -- LANDMASS -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void landmass_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color tile_color = (Color) { 0x20, 0x79, 0xF7, 0xFF };

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_UNDERWATER_CANYONS)) {
        tile_color = (Color) { 0x21, 0x36, 0x8F, 0xFF };
    } else if (hexa_cell_has_flag(cell, HEXAW_FLAG_MOUNTAIN)) {
        tile_color = (Color) { 0x90, 0x5F, 0x07, 0xFF };
    } else if (hexa_cell_has_flag(cell, HEXAW_FLAG_CANYONS)) {
        tile_color = (Color) { 0x93, 0x7E, 0x58, 0xFF };
        
    } else if (cell->altitude > 0) {
        tile_color = (Color) { 0xC8, 0x9B, 0x49, 0xFF };
    }


    DrawPoly(*((Vector2*) &target_shape->center), HEXAGON_SIDES_NB, target_shape->radius, 0.0f, tile_color);

    if (hexa_cell_has_flag(cell, HEXAW_FLAG_ISLES)) {
        DrawCircleV(*((Vector2*) &target_shape->center), target_shape->radius/2, (Color) { 0xC8, 0x9B, 0x49, 0xFF });   
    }
}

// -------------------------------------------------------------------------------------------------
static void landmass_seed(hexaworld_t *world) {
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            if (hexa_cell_has_flag(world->tiles[x] + y, HEXAW_FLAG_TELLURIC_RIDGE)) {
                world->tiles[x][y].altitude = 1;
            } else if (!hexa_cell_has_flag(world->tiles[x] + y, HEXAW_FLAG_TELLURIC_RIFT)){
                world->tiles[x][y].altitude = (rand() & LANDMASS_SEEDING_CHANCE) != 0;
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void landmass_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    u32 counter = 0u;

    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        counter += ((hexa_cell_t *) neighbors[i])->altitude;
    }

    cell->altitude = (counter > (DIRECTIONS_NB / 2));
}

// -------------------------------------------------------------------------------------------------
static void landmass_flag_gen(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    const u32 is_above_sea_level = cell->altitude;
    const u32 is_a_ridge = hexa_cell_has_flag(cell, HEXAW_FLAG_TELLURIC_RIDGE);
    const u32 is_a_rift = hexa_cell_has_flag(cell, HEXAW_FLAG_TELLURIC_RIFT);

    if (is_above_sea_level) {
        if (is_a_ridge) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_MOUNTAIN);
        } else if (is_a_rift) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_CANYONS);
        }
    } else {
        if (is_a_ridge) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_ISLES);
        } else if (is_a_rift) {
            hexa_cell_set_flag(cell, HEXAW_FLAG_UNDERWATER_CANYONS);
        }
    }
}

const layer_calls_t landmass_layer_calls = {
        .draw_func          = &landmass_draw,
        .seed_func          = &landmass_seed,
        .automaton_func     = &landmass_apply,
        .flag_gen_func      = &landmass_flag_gen, 
        .automaton_iter     = ITERATION_NB_LANDMASS,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE
};