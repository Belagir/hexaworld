/**
 * @file hexaworld_application.c
 * @author gabriel 
 * @brief definition file for the hexaworld application data.
 * @version 0.1
 * @date 2023-04-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <hexaworld_application.h>

#include <stdlib.h>
#include <time.h>

#include <raylib.h>

#include <colorpalette.h>
#include <endoftheline.h>
#include <hexagonparadigm.h>

#include "hexaworld/hexaworld.h"
#include "infopanel/infopanel.h"
#include "windowdivision/windowregion.h"

// -------------------------------------------------------------------------------------------------
// ---- FILE CONSTANTS -----------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

#define HEXAPP_WINDOW_TITLE "hexaworld" ///< Title of the raylib window.

/**
 * @brief Lists the registered window region in the application
 */
typedef enum registered_window_region_t {
    WINREGION_HEXAWORLD,
    WINREGION_TILEINFO,

    WINREGIONS_NUMBER
} registered_window_region_t;

// -------------------------------------------------------------------------------------------------
// ---- TYPE DEFINITIONS ---------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief World data and some collateral data for the main loop
 */
typedef struct hexaworld_application_data_t {
    hexaworld_t *hexaworld;
    hexaworld_layer_t current_layer;
    info_panel_t *linked_panel;
} hexaworld_application_data_t;

/**
 * @brief Defintion for the hexaworld raylib application data type.
 */
typedef struct hexaworld_raylib_app_handle_t {
    /// application-specific world data
    hexaworld_application_data_t hexaworld_data;

    /// pixel width of the window
    i32 window_width;
    /// pixel height of the window
    i32 window_height;

    /// window region information
    window_region_t *window_regions[WINREGIONS_NUMBER];
} hexaworld_raylib_app_handle_t;

// -------------------------------------------------------------------------------------------------
// ---- STATIC DATA --------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

static struct {
    hexaworld_raylib_app_handle_t real_app;
} module_data = { 0u };

static const f32 window_position_map[WINREGIONS_NUMBER][4u] = {
        // WINREGION_HEXAWORLD
        { 0.0f,  0.0f, 0.75f, 1.0f },
        // WINREGION_TILEINFO
        { 0.75f, 0.0f, 0.25f, 1.0f },
};

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DECLARATIONS --------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

static void application_end_of_the_line_destroy(void *raw_ptr_app);
/**
 * @brief (Re-)generates all the layers of a world.
 * 
 * @param world target world.
 */
static void generate_world(hexaworld_t *world);

static void winregion_hexaworld_on_refresh(vector_2d_cartesian_t target_dim, void *world_data);

static void winregion_hexaworld_on_click(vector_2d_cartesian_t region_dim, u32 x, u32 y, void *world_data);

static void winregion_infopanel_on_refresh(vector_2d_cartesian_t target_dim, void *info_panel_data);


// -------------------------------------------------------------------------------------------------
// ---- HEADER FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
hexaworld_raylib_app_handle_t * hexaworld_raylib_app_init(i32 random_seed, u32 window_width, u32 window_height, u32 world_width, u32 world_height) {
    hexaworld_raylib_app_handle_t *handle = &module_data.real_app;

    i32 real_seed = 0;

    // computing seed
    real_seed = random_seed;
    if (random_seed == 0) {
        srand(time(NULL));
        real_seed = rand();
    }

    end_of_the_line_register_call(application_end_of_the_line_destroy, &(module_data.real_app));

    // window dimensions
    handle->window_height = window_height;
    handle->window_width  = window_width;

    // raylib window
    InitWindow(handle->window_width, handle->window_height, HEXAPP_WINDOW_TITLE);
    if (!IsWindowReady()) {
        end_of_the_line(END_OF_THE_LINE_EXIT_RAYLIB_FAILED, "failure during application initialisation");
    }

    for (size_t i = 0u ; i < WINREGIONS_NUMBER ; i++) {
        handle->window_regions[i] = NULL;
    }

    // hexaworld allocation & initialisation of the companion data
    handle->hexaworld_data = (hexaworld_application_data_t) {
            .hexaworld = hexaworld_create_empty(world_width, world_height, real_seed),
            .current_layer = HEXAW_LAYER_WHOLE_WORLD,
            .linked_panel = info_panel_create(),
    };

    if ((!handle->hexaworld_data.hexaworld) || (!handle->hexaworld_data.linked_panel)) {
        end_of_the_line(END_OF_THE_LINE_EXIT_NO_MEMORY, "failure during application initialisation");
    }


    // assign window regions to some data
    handle->window_regions[WINREGION_HEXAWORLD] = window_region_create(
            window_position_map[WINREGION_HEXAWORLD],
            handle->window_width, 
            handle->window_height,
            &winregion_hexaworld_on_click,
            &winregion_hexaworld_on_refresh,
            (void *) &handle->hexaworld_data);
    handle->window_regions[WINREGION_TILEINFO] = window_region_create(
            window_position_map[WINREGION_TILEINFO], 
            handle->window_width, 
            handle->window_height,
            NULL,
            &winregion_infopanel_on_refresh,
            (void *) handle->hexaworld_data.linked_panel);
    
    for (size_t i = 0u ; i < WINREGIONS_NUMBER ; i++) {
        if (!handle->window_regions[i]) {
            end_of_the_line(END_OF_THE_LINE_EXIT_NO_MEMORY, "failure during application initialisation");
        }
    }

    info_panel_set_map_seed(handle->hexaworld_data.linked_panel, real_seed);

    if (handle->hexaworld_data.hexaworld) {
        // generate ALL the LAYERS !
        generate_world(handle->hexaworld_data.hexaworld);
    }
    
    return handle;
}

// -------------------------------------------------------------------------------------------------
void hexaworld_raylib_app_deinit(hexaworld_raylib_app_handle_t **hexapp) {
    if ((!hexapp) || (!(*hexapp))) {
        return;
    }

    for (size_t i = 0; i < WINREGIONS_NUMBER; i++) {
        window_region_destroy((*hexapp)->window_regions + i);
    }

    info_panel_destroy(&((*hexapp)->hexaworld_data.linked_panel));
    hexaworld_destroy(&((*hexapp)->hexaworld_data.hexaworld));

    if (IsWindowReady()) {
        CloseWindow();
    }

    (*hexapp)->window_height = 0u;
    (*hexapp)->window_width = 0u;

    (*hexapp) = NULL;
}

// -------------------------------------------------------------------------------------------------
void hexaworld_raylib_app_run(hexaworld_raylib_app_handle_t *hexapp, u32 target_fps) {
    i32 new_seed = 0;

    if (!IsWindowReady() || (!hexapp) || (!hexapp->hexaworld_data.hexaworld)) {
        return;
    }

    SetTargetFPS(target_fps);

    for (size_t i = 0; i < WINREGIONS_NUMBER; i++) {
        window_region_refresh(hexapp->window_regions[i]);
    }

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_SHIFT)) {
            new_seed = rand();
            hexaworld_reseed(hexapp->hexaworld_data.hexaworld, new_seed);
            generate_world(hexapp->hexaworld_data.hexaworld);

            info_panel_set_map_seed(hexapp->hexaworld_data.linked_panel, new_seed);
            info_panel_set_examined_cell(hexapp->hexaworld_data.linked_panel, NULL, 0u, 0u);

            window_region_notify_changed(hexapp->window_regions[WINREGION_HEXAWORLD]);
            window_region_notify_changed(hexapp->window_regions[WINREGION_TILEINFO]);

        } else if (IsKeyPressed(KEY_RIGHT)) {
            hexapp->hexaworld_data.current_layer = (hexapp->hexaworld_data.current_layer + 1u) % HEXAW_LAYERS_NUMBER;
            window_region_notify_changed(hexapp->window_regions[WINREGION_HEXAWORLD]);

        } else if (IsKeyPressed(KEY_LEFT)) {
            hexapp->hexaworld_data.current_layer = (hexapp->hexaworld_data.current_layer == 0)
                    ? HEXAW_LAYERS_NUMBER - 1u
                    : hexapp->hexaworld_data.current_layer - 1u;
            window_region_notify_changed(hexapp->window_regions[WINREGION_HEXAWORLD]);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (size_t i = 0; i < WINREGIONS_NUMBER; i++) {
                window_region_process_click(hexapp->window_regions[i], GetMouseX(), GetMouseY());
            }
            window_region_notify_changed(hexapp->window_regions[WINREGION_TILEINFO]);
        }

        for (size_t i = 0; i < WINREGIONS_NUMBER; i++) {
            window_region_refresh(hexapp->window_regions[i]);
        }

        BeginDrawing();
        ClearBackground(AS_RAYLIB_COLOR(COLOR_BLAND));
        for (size_t i = 0; i < WINREGIONS_NUMBER; i++) {
            window_region_draw(hexapp->window_regions[i]);
        }
        EndDrawing();
    }
}

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void application_end_of_the_line_destroy(void *raw_ptr_app) {
    hexaworld_raylib_app_handle_t *ptr_app = (hexaworld_raylib_app_handle_t *) raw_ptr_app;

    hexaworld_raylib_app_deinit(&ptr_app);
}

// -------------------------------------------------------------------------------------------------
static void generate_world(hexaworld_t *world) {
    hexaworld_raze(world);
    
    for (size_t i_layer = 0u ; i_layer < HEXAW_LAYERS_NUMBER ; i_layer++) {
        hexaworld_genlayer(world, i_layer);
    }
}

// -------------------------------------------------------------------------------------------------
static void winregion_hexaworld_on_refresh(vector_2d_cartesian_t target_dim, void *world_data) {
    hexaworld_application_data_t *hexaworld_data = (hexaworld_application_data_t *) world_data;

    f32 target_rectangle[4u] = { 0.0f, 0.0f, target_dim.v, target_dim.w };
    
    hexaworld_draw(hexaworld_data->hexaworld, hexaworld_data->current_layer, target_rectangle);
}

// -------------------------------------------------------------------------------------------------
static void winregion_hexaworld_on_click(vector_2d_cartesian_t region_dim, u32 x, u32 y, void *world_data) {
    hexaworld_application_data_t *hexaworld_data = (hexaworld_application_data_t *) world_data;
    hexa_cell_t *clicked_cell = NULL;
    u32 array_x = 0u;
    u32 array_y = 0u;

    f32 target_rectangle[4u] = { 0.0f, 0.0f, region_dim.v, region_dim.w };

    clicked_cell = hexaworld_tile_at(hexaworld_data->hexaworld, x, y, target_rectangle, &array_x, &array_y);

    info_panel_set_examined_cell(hexaworld_data->linked_panel, clicked_cell, array_x, array_y);
}

// -------------------------------------------------------------------------------------------------
static void winregion_infopanel_on_refresh(vector_2d_cartesian_t target_dim, void *info_panel_data) {
    info_panel_t *panel = (info_panel_t *) info_panel_data;

    info_panel_draw(panel);
}
