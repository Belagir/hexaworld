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

#include "hexaworld/hexaworld.h"

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
} hexaworld_raylib_app_handle_t;

// -------------------------------------------------------------------------------------------------
// ---- STATIC DATA --------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

static struct {
    hexaworld_raylib_app_handle_t real_app;
} module_data = { 0u };

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DECLARATIONS --------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Draws a layer of an hexaworld to a render texture.
 * 
 * @param[in] world world to draw.
 * @param[out] target target render texture
 * @param[in] target_rectangle target rectangle as the aggregation of the topleft coordinate and the two lengths of the rectangle
 * @param[in] layer layer to draw to the texture
 */
static void draw_hexmap_to_texture(hexaworld_t *world, RenderTexture2D *target, f32 target_rectangle[4u], hexaworld_layer_t layer);

/**
 * @brief (Re-)generates all the layers of a world.
 * 
 * @param world target world.
 */
static void generate_world(hexaworld_t *world);

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
    f32 world_rectangle[4u] = { 0u };
    RenderTexture2D world_buffer = { 0u };
    u32 layer_counter = 0u;
    u32 layer_changed = 1u;

    if (!IsWindowReady() || (!hexapp) || (!hexapp->hexaworld)) {
        return;
    }

    SetTargetFPS(target_fps);

    world_rectangle[2u] = (f32) WORLD_TEXTURE_BUFFER_WIDTH;
    world_rectangle[3u] = (f32) WORLD_TEXTURE_BUFFER_HEIGHT;

    world_buffer = LoadRenderTexture(world_rectangle[2u], world_rectangle[3u]);

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
            draw_hexmap_to_texture(hexapp->hexaworld, &world_buffer, world_rectangle, layer_counter);
            layer_changed = 0u;
        }

        BeginDrawing();
        DrawTexturePro(
                world_buffer.texture, 
                (Rectangle) { world_rectangle[0u], world_rectangle[1u], world_rectangle[2u], world_rectangle[3u] }, 
                (Rectangle) { 0.0f, 0.0f, GetScreenWidth(), GetScreenHeight() },
                (Vector2)   { 0.0f, 0.0f },
                0.0f,
                WHITE
        );
        EndDrawing();
    }

    UnloadRenderTexture(world_buffer);
}

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void draw_hexmap_to_texture(hexaworld_t *world, RenderTexture2D *target, f32 target_rectangle[4u], hexaworld_layer_t layer) {
    BeginTextureMode(*target);
    ClearBackground(WHITE);
    
    hexaworld_draw(world, layer, target_rectangle);

    EndTextureMode();

    GenTextureMipmaps(&(target->texture));
    SetTextureFilter(target->texture, TEXTURE_FILTER_BILINEAR);
}

// -------------------------------------------------------------------------------------------------
static void generate_world(hexaworld_t *world) {
    hexaworld_raze(world);
    
    for (size_t i_layer = 0u ; i_layer < HEXAW_LAYERS_NUMBER ; i_layer++) {
        hexaworld_genlayer(world, i_layer);
    }
}
