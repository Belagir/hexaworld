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

#include "hexaworld.h"

// -------------------------------------------------------------------------------------------------
// ---- FILE CONSTANTS -----------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

#define HEXAPP_WINDOW_TITLE "hexaworld" ///< Title of the raylib window.

// -------------------------------------------------------------------------------------------------
// ---- TYPE DEFINITIONS ---------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Defintion for the hexaworld raylib application data type.
 */
typedef struct hexaworld_raylib_app_t {
    /// pointer to a world data
    hexaworld_t *hexaworld;

    /// pixel width of the window
    i32 window_width;
    /// pixel height of the window
    i32 window_height;
} hexaworld_raylib_app_t;

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DECLARATIONS --------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Draws a layer of an hexaworld to a render texture.
 * 
 * @param[in] world world to draw.
 * @param[out] target target render texture
 * @param[in] window_rectangle target rectangle as the aggregation of the topleft coordinate and the two lengths of the rectangle
 * @param[in] layer layer to draw to the texture
 */
static void draw_hexmap_to_texture(hexaworld_t *world, RenderTexture2D *target, f32 window_rectangle[4u], hexaworld_layer_t layer);

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
hexaworld_raylib_app_t *hexaworld_raylib_app_create(u32 window_width, u32 window_height, u32 world_width, u32 world_height) {
    hexaworld_raylib_app_t *hexapp = NULL;

    hexapp = malloc(sizeof(*hexapp));
    if (!hexapp) {
        return NULL;
    }

    *hexapp = (hexaworld_raylib_app_t) { 0u };
    hexapp->hexaworld = hexaworld_create_empty(world_width, world_height);
    hexapp->window_height = window_height;
    hexapp->window_width = window_width;
    
    return hexapp;
}

// -------------------------------------------------------------------------------------------------
void hexaworld_raylib_app_destroy(hexaworld_raylib_app_t **hexapp) {
    hexaworld_destroy(&(*hexapp)->hexaworld);

    if (IsWindowReady()) {
        CloseWindow();
    }

    (*hexapp)->window_height = 0u;
    (*hexapp)->window_width = 0u;

    free((*hexapp));
    (*hexapp) = NULL;
}

// -------------------------------------------------------------------------------------------------
void hexaworld_raylib_app_init(hexaworld_raylib_app_t *hexapp, i32 random_seed) {
    InitWindow(hexapp->window_width, hexapp->window_height, HEXAPP_WINDOW_TITLE);
    srand(random_seed);

    // generate ALL the LAYERS !
    generate_world(hexapp->hexaworld);
}

// -------------------------------------------------------------------------------------------------
void hexaworld_raylib_app_run(hexaworld_raylib_app_t *hexapp, u32 target_fps) {
    f32 window_rectangle[4u] = { 0u };
    RenderTexture2D world_buffer = { 0u };
    u32 layer_counter = 0u;
    u32 layer_changed = 1u;

    if (!IsWindowReady()) {
        return;
    }

    SetTargetFPS(target_fps);

    window_rectangle[0u] = 0.0f;
    window_rectangle[1u] = 0.0f;
    window_rectangle[2u] = (f32) GetScreenWidth();
    window_rectangle[3u] = (f32) GetScreenHeight();

    world_buffer = LoadRenderTexture(window_rectangle[2u], window_rectangle[3u]);

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
            draw_hexmap_to_texture(hexapp->hexaworld, &world_buffer, window_rectangle, layer_counter);
            layer_changed = 0u;
        }

        BeginDrawing();
        DrawTexturePro(
                world_buffer.texture, 
                (Rectangle) { 0.0f, 0.0f, world_buffer.texture.width, -world_buffer.texture.height }, 
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
static void draw_hexmap_to_texture(hexaworld_t *world, RenderTexture2D *target, f32 window_rectangle[4u], hexaworld_layer_t layer) {
    BeginTextureMode(*target);
    ClearBackground(WHITE);
    hexaworld_draw(world, layer, window_rectangle);
    EndTextureMode();
    GenTextureMipmaps(&(target->texture));
    SetTextureFilter(target->texture, TEXTURE_FILTER_TRILINEAR);
}

// -------------------------------------------------------------------------------------------------
static void generate_world(hexaworld_t *world) {
    hexaworld_raze(world);
    
    for (size_t i_layer = 0u ; i_layer < HEXAW_LAYERS_NUMBER ; i_layer++) {
        hexaworld_genlayer(world, i_layer);
    }
}
