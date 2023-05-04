
#include "hexaworldcomponents.h"

#include <math.h>

#include <raylib.h>

#include <colorpalette.h>

#define ITERATION_NB_CLOUD_COVER (10u)    ///< number of automaton iteration for the cloud cover layer

// -------------------------------------------------------------------------------------------------
// -- CLOUD COVER -----------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void cloud_cover_draw(hexa_cell_t *cell, hexagon_shape_t *target_shape) {
    Color base_color = AS_RAYLIB_COLOR(COLOR_CERULEAN);
    
    base_color.a = (u8) (cell->cloud_cover * 255u);
    draw_hexagon(target_shape, FROM_RAYLIB_COLOR(base_color), 1.0f, DRAW_HEXAGON_FILL);
    DrawCircleV(
            *((Vector2 *) &(target_shape->center)), 
            (2*target_shape->radius/3) * cell->precipitations, 
            AS_RAYLIB_COLOR(COLOR_GRAY)
    );
    if (cell->altitude > 0) {
        draw_hexagon(target_shape, COLOR_LEATHER, 0.83f, DRAW_HEXAGON_LINES);
    }
}

// -------------------------------------------------------------------------------------------------
static void cloud_cover_seed(hexaworld_t *world) {
    for (size_t x = 0u ; x < world->width ; x++) {
        for (size_t y = 0u ; y < world->height ; y++) {
            world->tiles[x][y].cloud_cover = (f32) (world->tiles[x][y].altitude <= 0);
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void cloud_cover_apply(void *target_cell, void *neighbors[DIRECTIONS_NB]) {
    hexa_cell_t *cell = (hexa_cell_t *) target_cell;

    const f32 step_angle = PI_T_2 / (f32) DIRECTIONS_NB;
    const ratio_t cell_abv_sea_altitude_ratio = MIN((f32) (cell->altitude * (cell->altitude > 0)) / ((f32) ALTITUDE_MAX / 2), 1.0f); 

    f32 diff_angle_of_neighbors[DIRECTIONS_NB] = { 0.0f };
    ratio_t cell_given_clouds[DIRECTIONS_NB] = { 0.0f };
    f32 mirrored_angle = 0.0f;

    if (cell->altitude <= 0) {
        return;
    }

    // computing the neighboring cell's wind angle subjective directions
    hexa_cell_direction_of_surrounding_angles((hexa_cell_t **) neighbors, (void *)(&((hexa_cell_t *) NULL)->winds_vector.angle), diff_angle_of_neighbors);
    // diff_angle_of_neighbors contains the angles of the wind vectors in relation of their position (0 is pointing to our cell, PI is pointing the other way)

    // transform angles to their ratio 
    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        // putting the angle in a ""positive"" state of mind
        mirrored_angle = fmodf(diff_angle_of_neighbors[i] + PI, PI_T_2);
        mirrored_angle += PI_T_2 * (mirrored_angle < 0.0f);
        // mirror of the angle over the x-axis
        mirrored_angle = (mirrored_angle > PI)
                ? -(mirrored_angle - PI_T_2)
                : mirrored_angle;
                
        if ((mirrored_angle > (step_angle)) || float_equal(mirrored_angle, step_angle, 1u)) {
            // if the angle is pointing to another cell
            cell_given_clouds[i] = 0.0f;
        } else {
            cell_given_clouds[i] = (mirrored_angle / (step_angle)) * ((hexa_cell_t *) neighbors[i])->cloud_cover;
        }
    }

    // applying the humidity comming from other cells to our cell
    cell->cloud_cover = 0.0f;
    for (size_t i = 0u ; i < DIRECTIONS_NB ; i++) {
        cell->cloud_cover += cell_given_clouds[i];
    }
    cell->cloud_cover = MIN(cell->cloud_cover, 1.0f);

    // making the clouds rain a bit depending of the altitude
    cell->precipitations = 
            ((cell->cloud_cover + cell_abv_sea_altitude_ratio) > 1.0f) 
            * fabs(cell->cloud_cover - cell_abv_sea_altitude_ratio);
    // cell->cloud_cover -= cell->precipitations;
    // cell->cloud_cover *= cell->winds_vector.magnitude;
}

const layer_calls_t cloud_cover_layer_calls = {
        .draw_func          = &cloud_cover_draw,
        .seed_func          = &cloud_cover_seed,
        .automaton_func     = &cloud_cover_apply,
        .flag_gen_func      = NULL, 
        .automaton_iter     = ITERATION_NB_CLOUD_COVER,
        .iteration_flavour  = LAYER_GEN_ITERATE_ABSOLUTE    
};