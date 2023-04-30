
#ifndef __WINDOWREGION_H__
#define __WINDOWREGION_H__

#include <unstandard.h>
#include <raylib.h>

typedef void (*on_region_click_func_t)(u32 relative_x, u32 relative_y, void *data);
typedef void (*on_region_refreshed_func_t)(RenderTexture2D target_texture, f32 coords_rectangle[4u], void *data);

typedef struct window_region_t {
    f32 coords_rectangle[4u];
    
    on_region_click_func_t on_click_f;
    on_region_refreshed_func_t on_refresh_f;

    RenderTexture2D buffer_rendertexture;
    void *related_data;
} window_region_t;

#endif
