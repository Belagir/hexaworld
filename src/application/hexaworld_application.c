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
#include <raylib.h>

#include <colorpalette.h>

#include "hexaworld/hexaworld.h"
#include "infopanel/infopanel.h"
#include "windowdivision/windowregion.h"

// -------------------------------------------------------------------------------------------------
// ---- FILE CONSTANTS -----------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

#define HEXAPP_WINDOW_TITLE "hexaworld" ///< Title of the raylib window.

// -------------------------------------------------------------------------------------------------
// ---- TYPE DEFINITIONS ---------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief World data and some collateral data for the main loop
 */
typedef struct hexaworld_application_data_t {
    hexaworld_t *hexaworld;
    hexaworld_layer_t current_layer;
    u32 layer_changed;
} hexaworld_application_data_t;

/**
 * @brief Defintion for the hexaworld raylib application data type.
 */
typedef struct hexaworld_raylib_app_handle_t {
    /// application-specific world data
    hexaworld_application_data_t hexaworld_data;

    /// pointer to the info panel data
    info_panel_t *info_panel;

    /// pixel width of the window
    i32 window_width;
    /// pixel height of the window
    i32 window_height;

    /// window region information
    window_region_t window_regions[WINREGIONS_NUMBER];
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

/**
 * @brief (Re-)generates all the layers of a world.
 * 
 * @param world target world.
 */
static void generate_world(hexaworld_t *world);

static void winregion_hexaworld_on_refresh(vector_2d_cartesian_t target_dim, void *world_data);

static void winregion_hexaworld_on_click(vector_2d_cartesian_t region_dim, u32 x, u32 y, void *world_data);

static void winregion_tileinfo_on_refresh(vector_2d_cartesian_t target_dim, void *tile_info_data);

// -------------------------------------------------------------------------------------------------
// ---- HEADER FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
hexaworld_raylib_app_handle_t * hexaworld_raylib_app_init(i32 random_seed, u32 window_width, u32 window_height, u32 world_width, u32 world_height) {
    // window dimensions
    module_data.real_app.window_height = window_height;
    module_data.real_app.window_width  = window_width;

    // hexaworld allocation & initialisation of the companion data
    module_data.real_app.hexaworld_data = (hexaworld_application_data_t) {
            .hexaworld = hexaworld_create_empty(world_width, world_height),
            .current_layer = HEXAW_LAYER_WHOLE_WORLD,
            .layer_changed = 0u,
    };

    // raylib window
    InitWindow(module_data.real_app.window_width, module_data.real_app.window_height, HEXAPP_WINDOW_TITLE);
    // random number generator
    srand(random_seed);

    if (module_data.real_app.hexaworld_data.hexaworld) {
        // generate ALL the LAYERS !
        generate_world(module_data.real_app.hexaworld_data.hexaworld);
    }

    // assign window regions to some data
    window_region_init(
            module_data.real_app.window_regions + WINREGION_HEXAWORLD, 
            window_position_map[WINREGION_HEXAWORLD], 
            module_data.real_app.window_width, 
            module_data.real_app.window_height,
            &winregion_hexaworld_on_click,
            &winregion_hexaworld_on_refresh,
            (void *) &module_data.real_app.hexaworld_data);
    window_region_init(
            module_data.real_app.window_regions + WINREGION_TILEINFO,
            window_position_map[WINREGION_TILEINFO],
            module_data.real_app.window_width, 
            module_data.real_app.window_height,
            NULL,
            &winregion_tileinfo_on_refresh,
            (void *) module_data.real_app.info_panel
    );

    return &(module_data.real_app);
}

// -------------------------------------------------------------------------------------------------
void hexaworld_raylib_app_deinit(hexaworld_raylib_app_handle_t **hexapp) {
    for (size_t i = 0; i < WINREGIONS_NUMBER; i++) {
        window_region_deinit((*hexapp)->window_regions + i);
    }

    hexaworld_destroy(&((*hexapp)->hexaworld_data.hexaworld));
    info_panel_destroy(&((*hexapp)->info_panel));

    if (IsWindowReady()) {
        CloseWindow();
    }

    (*hexapp)->window_height = 0u;
    (*hexapp)->window_width = 0u;

    (*hexapp) = NULL;
}

// -------------------------------------------------------------------------------------------------
void hexaworld_raylib_app_run(hexaworld_raylib_app_handle_t *hexapp, u32 target_fps) {

    if (!IsWindowReady() || (!hexapp) || (!hexapp->hexaworld_data.hexaworld)) {
        return;
    }

    SetTargetFPS(target_fps);

    for (size_t i = 0; i < WINREGIONS_NUMBER; i++) {
        window_region_refresh(hexapp->window_regions + i);
    }

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_SHIFT)) {
            generate_world(hexapp->hexaworld_data.hexaworld);
            hexapp->hexaworld_data.layer_changed = 1u;

        } else if (IsKeyPressed(KEY_RIGHT)) {
            hexapp->hexaworld_data.current_layer = (hexapp->hexaworld_data.current_layer + 1u) % HEXAW_LAYERS_NUMBER;
            hexapp->hexaworld_data.layer_changed = 1u;

        } else if (IsKeyPressed(KEY_LEFT)) {
            hexapp->hexaworld_data.current_layer = (hexapp->hexaworld_data.current_layer == 0)
                    ? HEXAW_LAYERS_NUMBER - 1u
                    : hexapp->hexaworld_data.current_layer - 1u;
            hexapp->hexaworld_data.layer_changed = 1u;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (size_t i = 0; i < WINREGIONS_NUMBER; i++) {
                window_region_process_click(hexapp->window_regions + i, GetMouseX(), GetMouseY());
            }
        }

        if (hexapp->hexaworld_data.layer_changed) {
            window_region_refresh(hexapp->window_regions + WINREGION_HEXAWORLD);
             hexapp->hexaworld_data.layer_changed = 0u;
        }

        BeginDrawing();
        ClearBackground(AS_RAYLIB_COLOR(COLOR_BLAND));
        for (size_t i = 0; i < WINREGIONS_NUMBER; i++) {
            window_region_draw(hexapp->window_regions + i);
        }
        EndDrawing();
    }
}

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

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

    f32 target_rectangle[4u] = { 0.0f, 0.0f, region_dim.v, region_dim.w };

    clicked_cell = hexaworld_tile_at(hexaworld_data->hexaworld, x, y, target_rectangle);

    if (!clicked_cell) {
        return;
    }

    // TODO
}

// -------------------------------------------------------------------------------------------------
static void winregion_tileinfo_on_refresh(vector_2d_cartesian_t target_dim, void *tile_info_data) {
    ClearBackground(AS_RAYLIB_COLOR(COLOR_WHITE));
}
