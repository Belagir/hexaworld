
#ifndef __COLORPALETTE_H__
#define __COLORPALETTE_H__

#include <raylib.h>
#include <unstandard.h>

#define AS_RAYLIB_COLOR(_c) ((Color) { .r = (((_c) & 0xFF000000) >> 24u), .g = (((_c) & 0x00FF0000) >> 16u), .b = (((_c) & 0x0000FF00) >> 8u), .a = (((_c) & 0x000000FF)) })


#define COLOR_WHITE         (0xFFFFFFFF)
#define COLOR_ICE_BLUE      (0xDAF7FFFF)

#define COLOR_TOMATO        (0xE43625FF)
#define COLOR_BUTTERSCOTCH  (0xE49625FF)
#define COLOR_AQUA_GREEN    (0x25E496FF)

#define COLOR_DUSK_BLUE     (0x2E548FFF)
#define COLOR_CERULEAN      (0x2B93DFFF)
#define COLOR_SAND_BROWN    (0xD8B168FF)
#define COLOR_WHEAT         (0xFFD07BFF)
#define COLOR_LEATHER       (0x9E6C2EFF)
#define COLOR_TAUPE         (0xA39272FF)

#define COLOR_DARKISH_RED   (0xB5050AFF)
#define COLOR_AZURE         (0x34A8FFFF)

#define COLOR_GRAY          (0xA1A1A1FF)
#define COLOR_GRAYISH_BROWN (0x585041FF)

#define COLOR_LEAFY_GREEN   (0x4FBB3AFF)

#endif
