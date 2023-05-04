/**
 * @file colorpalette.h
 * @author gabriel 
 * @brief Declares some colors and color conversion macros for raylib
 * @version 0.1
 * @date 2023-04-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __COLORPALETTE_H__
#define __COLORPALETTE_H__

#define AS_RAYLIB_COLOR(_c) ((Color) { .r = (((_c) & 0xFF000000) >> 24u), .g = (((_c) & 0x00FF0000) >> 16u), .b = (((_c) & 0x0000FF00) >> 8u), .a = (((_c) & 0x000000FF)) })    ///< converts a u32-coded color to a raylib structure (needs raylib.h)

#define FROM_RAYLIB_COLOR(_C) (((_C).r << 24u) | ((_C).g << 16u) | ((_C).b << 8u) | (_C).a)     ///< converts a raylib color structure to a u32-coded color (needs raylib.h)

#define COLOR_NO_COLOR      (0x00000000)    ///< actually no color

#define COLOR_WHITE         (0xFFFFFFFF)    ///< white color
#define COLOR_BLACK         (0x000000FF)    ///< black color
#define COLOR_GRAY          (0xA1A1A1FF)    ///< gray color
#define COLOR_GRAYISH_BROWN (0x585041FF)    ///< dirty gray color

#define COLOR_DUSK          (0x4A4E77FF)    ///< gray-blue color
#define COLOR_DUSK_BLUE     (0x2E548FFF)    ///< deep blue color
#define COLOR_AZURE         (0x34A8FFFF)    ///< azure color
#define COLOR_CERULEAN      (0x2B93DFFF)    ///< vibrant blue color
#define COLOR_ICE_BLUE      (0xDAF7FFFF)    ///< almost white ice blue color

#define COLOR_AQUA_GREEN    (0x25E496FF)    ///< green-ish turquoise-ish color
#define COLOR_LEAFY_GREEN   (0x4FBB3AFF)    ///< nice green color
#define COLOR_TREE_GREEN    (0x197E15FF)    ///< dark green color

#define COLOR_WHEAT         (0xFFD07BFF)    ///< pale yellow-brownish color
#define COLOR_SAND_BROWN    (0xD8B168FF)    ///< sand color
#define COLOR_BLAND         (0xCCC8ABFF)    ///< beige... color

#define COLOR_LEATHER       (0x9E6C2EFF)    ///< brown color
#define COLOR_TAUPE         (0xA39272FF)    ///< grey-a-tad-brown color

#define COLOR_DARKISH_RED   (0xB5050AFF)    ///< dark red color
#define COLOR_TOMATO        (0xE43625FF)    ///< vibrant red color
#define COLOR_TANGERINE     (0xFF8E20FF)    ///< shy orange

#endif
