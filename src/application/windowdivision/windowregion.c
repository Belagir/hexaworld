
#include "windowregion.h"

#include <raylib.h>

#include <colorpalette.h>

// -------------------------------------------------------------------------------------------------
void window_region_init(
            window_region_t *w_region,
            const f32 ratio_coords_rectangle[4u],
            u32 window_width,
            u32 window_height,
            on_region_click_func_t on_click_f,
            on_region_refreshed_func_t on_refresh_f,
            void *related_data) {
    
    (*w_region) = (window_region_t) { 0u };

    // x
    w_region->px_coords_rectangle[0u] = ratio_coords_rectangle[0u] * window_width,
    // y
    w_region->px_coords_rectangle[1u] = ratio_coords_rectangle[1u] * window_height,
    // width
    w_region->px_coords_rectangle[2u] = ratio_coords_rectangle[2u] * window_width,
    // height
    w_region->px_coords_rectangle[3u] = ratio_coords_rectangle[3u] * window_height,

    w_region->on_click_f = on_click_f;
    w_region->on_refresh_f = on_refresh_f;

    w_region->flag_changed = 1u;

    w_region->buffer_rendertexture = LoadRenderTexture(w_region->px_coords_rectangle[2u], w_region->px_coords_rectangle[3u]);

    w_region->related_data = related_data;
}

// -------------------------------------------------------------------------------------------------
void window_region_refresh(window_region_t *w_region) {
    if ((!w_region) || (!w_region->on_refresh_f) || (!w_region->flag_changed)) {
        return;
    }

    BeginTextureMode(w_region->buffer_rendertexture);
    ClearBackground(AS_RAYLIB_COLOR(COLOR_NO_COLOR));

    w_region->on_refresh_f(
            (vector_2d_cartesian_t) {
                    w_region->buffer_rendertexture.texture.width,
                    w_region->buffer_rendertexture.texture.height,
            }, 
            w_region->related_data
    );
    EndTextureMode();

    GenTextureMipmaps(&(w_region->buffer_rendertexture.texture));
    SetTextureFilter(w_region->buffer_rendertexture.texture, TEXTURE_FILTER_BILINEAR);

    w_region->flag_changed = 0u;
}

// -------------------------------------------------------------------------------------------------
void window_region_draw(window_region_t *w_region) {
    if (!w_region) {
        return;
    }

    DrawTexturePro(
                w_region->buffer_rendertexture.texture, 
                (Rectangle) { 0.0f, 0.0f, w_region->buffer_rendertexture.texture.width, -w_region->buffer_rendertexture.texture.height }, 
                (Rectangle) { w_region->px_coords_rectangle[0u], w_region->px_coords_rectangle[1u], w_region->px_coords_rectangle[2u], w_region->px_coords_rectangle[3u] },
                (Vector2)   { 0.0f, 0.0f },
                0.0f,
                WHITE
    );
}

// -------------------------------------------------------------------------------------------------
void window_region_process_click(window_region_t *w_region, i32 x, i32 y) {
    if ((!w_region) || (!w_region->on_click_f)) {
        return;
    }

    vector_2d_cartesian_t region_dim = (vector_2d_cartesian_t) { w_region->px_coords_rectangle[2u], w_region->px_coords_rectangle[3u] };

    if ((x >= w_region->px_coords_rectangle[0u]) 
            && (x < (w_region->px_coords_rectangle[0u] + w_region->px_coords_rectangle[2u]))
            && (y >= w_region->px_coords_rectangle[1u]) 
            && (y < (w_region->px_coords_rectangle[1u] + w_region->px_coords_rectangle[3u]))) {
        w_region->on_click_f(region_dim, (u32) (x - w_region->px_coords_rectangle[0u]), (u32) (y - w_region->px_coords_rectangle[1u]), w_region->related_data);
    }
}

// -------------------------------------------------------------------------------------------------
void window_region_deinit(window_region_t *w_region) {
    if (!w_region) {
        return;
    }

    UnloadRenderTexture(w_region->buffer_rendertexture);
}

// -------------------------------------------------------------------------------------------------
void window_region_notify_changed(window_region_t *w_region) {
    w_region->flag_changed = 1u;
}
