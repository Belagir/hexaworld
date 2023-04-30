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
#include "windowdivision/windowregion.h"

// -------------------------------------------------------------------------------------------------
// ---- FILE CONSTANTS -----------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

#define HEXAPP_WINDOW_TITLE "hexaworld" ///< Title of the raylib window.
#define WORLD_TEXTURE_BUFFER_WIDTH  2048u   ///< world render texture pixel x-axis dimension
#define WORLD_TEXTURE_BUFFER_HEIGHT 2048u   ///< world render texture pixel y-axis dimension

// -------------------------------------------------------------------------------------------------
// ---- TYPE DEFINITIONS ---------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Defintion for the hexaworld raylib application data type.
 */
typedef struct hexaworld_raylib_app_handle_t {
    /// pointer to a world data
    hexaworld_t *hexaworld;

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
        { 0.0f, 0.0f, 0.75f, 1.0f },
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

static void winregion_hexaworld_draw(vector_2d_cartesian_t target_dim, void *world_data);

// -------------------------------------------------------------------------------------------------
// ---- HEADER FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
hexaworld_raylib_app_handle_t * hexaworld_raylib_app_init(i32 random_seed, u32 window_width, u32 window_height, u32 world_width, u32 world_height) {
    module_data.real_app.window_height = window_height;
    module_data.real_app.window_width  = window_width;

    module_data.real_app.hexaworld = hexaworld_create_empty(world_width, world_height);

    InitWindow(module_data.real_app.window_width, module_data.real_app.window_height, HEXAPP_WINDOW_TITLE);
    srand(random_seed);

    if (module_data.real_app.hexaworld) {
        // generate ALL the LAYERS !
        generate_world(module_data.real_app.hexaworld);
    }

    // assign window regions to some data
    window_region_init(
            module_data.real_app.window_regions + WINREGION_HEXAWORLD, 
            window_position_map[WINREGION_HEXAWORLD], 
            module_data.real_app.window_width, 
            module_data.real_app.window_height,
            NULL,
            &winregion_hexaworld_draw,
            (void *) module_data.real_app.hexaworld);

    return &(module_data.real_app);
}

// -------------------------------------------------------------------------------------------------
void hexaworld_raylib_app_deinit(hexaworld_raylib_app_handle_t **hexapp) {
    hexaworld_destroy(&(*hexapp)->hexaworld);

    if (IsWindowReady()) {
        CloseWindow();
    }

    (*hexapp)->window_height = 0u;
    (*hexapp)->window_width = 0u;

    (*hexapp) = NULL;
}


// -------------------------------------------------------------------------------------------------
void hexaworld_raylib_app_run(hexaworld_raylib_app_handle_t *hexapp, u32 target_fps) {
    RenderTexture2D world_buffer = { 0u };
    u32 layer_counter = 0u;
    u32 layer_changed = 1u;

    if (!IsWindowReady() || (!hexapp) || (!hexapp->hexaworld)) {
        return;
    }

    SetTargetFPS(target_fps);

    layer_counter = HEXAW_LAYER_WHOLE_WORLD;
    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_SHIFT)) {
            generate_world(hexapp->hexaworld);
            layer_changed = 1u;
        } else if (IsKeyPressed(KEY_RIGHT)) {
            layer_counter = (layer_counter + 1u) % HEXAW_LAYERS_NUMBER;
            layer_changed = 1u;
        } else if (IsKeyPressed(KEY_LEFT)) {
            if (layer_counter == 0) {
                layer_counter = HEXAW_LAYERS_NUMBER - 1u;
            } else {
                layer_counter = layer_counter - 1u;
            }
            layer_changed = 1u;
        }

        if (layer_changed) {
            window_region_refresh(hexapp->window_regions + WINREGION_HEXAWORLD);
            layer_changed = 0u;
        }

        BeginDrawing();
        ClearBackground(AS_RAYLIB_COLOR(COLOR_BLAND));
        window_region_draw(hexapp->window_regions + WINREGION_HEXAWORLD);
        EndDrawing();
    }

    UnloadRenderTexture(world_buffer);
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
static void winregion_hexaworld_draw(vector_2d_cartesian_t target_dim, void *world_data) {
    hexaworld_t *hexaworld = (hexaworld_t *) world_data;

    f32 target_rectangle[4u] = {
            0.0f,
            0.0f,
            target_dim.v,
            target_dim.w,
    };
    
    hexaworld_draw(world_data, HEXAW_LAYER_WHOLE_WORLD, target_rectangle);
}