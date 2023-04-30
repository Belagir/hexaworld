
#ifndef __WINDOWREGION_H__
#define __WINDOWREGION_H__

#include <unstandard.h>
#include <raylib.h>

typedef enum registered_window_region_t {
    WINREGION_HEXAWORLD,

    WINREGIONS_NUMBER
} registered_window_region_t;

typedef void (*on_region_click_func_t)(u32 relative_x, u32 relative_y, void *data);
typedef void (*on_region_refreshed_func_t)(RenderTexture2D target_texture, f32 px_coords_rectangle[4u], void *data);

typedef struct window_region_t {
    f32 px_coords_rectangle[4u];
    
    on_region_click_func_t on_click_f;
    on_region_refreshed_func_t on_refresh_f;
    void *related_data;

    RenderTexture2D buffer_rendertexture;
} window_region_t;

void window_region_init(
            window_region_t *w_region,
            const f32 ratio_coords_rectangle[4u],
            u32 window_width,
            u32 window_height,
            on_region_click_func_t on_click_f,
            on_region_refreshed_func_t on_refresh_f,
            void *related_data);

#endif
