/**
 * @file hexagonparadigm.h
 * @author gabriel 
 * @brief The hexagon is the start and the end. This definition file contains some functions and data types needed to work with hexagons.
 * @version 0.1
 * @date 2023-04-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __HEXAGONPARADIGM_H__
#define __HEXAGONPARADIGM_H__

#include <unstandard.h>

#define HEXAGON_SIDES_NB (6u)     ///< number of sides of an hexagon. tough.

/**
 * @brief Possible directions from a hexagonal cell to another. Ordered along the unit circle.
 */
typedef enum cell_direction_t {
    DIRECTION_E,    ///< Right cell
    DIRECTION_SE,   ///< Bottom right cell
    DIRECTION_SW,   ///< Bottom left cell
    DIRECTION_W,    ///< Left cell
    DIRECTION_NW,   ///< Top left cell
    DIRECTION_NE,   ///< Top right cell
    DIRECTIONS_NB,  ///< Number of possible directions
} cell_direction_t;

/**
 * @brief Possible flags held in a cell. (They claim they're innocent)
 */
typedef enum hexaworld_cell_flag_t {
    HEXAW_FLAG_TELLURIC_RIDGE,      ///< Two plates are clashing here, and forming mountains
    HEXAW_FLAG_TELLURIC_RIFT,       ///< Two plates are growing from the ocean's floor, forming a deep crevasse

    HEXAW_FLAG_MOUNTAIN,            ///< Some mountains are forming here, above the water
    HEXAW_FLAG_ISLES,               ///< Some isles have been created by a tectonic force
    HEXAW_FLAG_CANYONS,             ///< Some old tectonic event or river dug a long path here
    HEXAW_FLAG_UNDERWATER_CANYONS,  ///< Tectonic forces are at work here and create some heavy drop on the ocean floor

    HEXAW_FLAG_MEANDERS,            ///< A river here twists and turns, creating a marsh, a bog, or maybe a swamp
    HEXAW_FLAG_WATERFALLS,          ///< Great waterfalls run down the landscape here
    HEXAW_FLAG_RIVER_MOUTH,         ///< The river on the tile goes directly to an ocean or sea 
    HEXAW_FLAG_LAKE,                ///< a lake has formed here

    HEXAW_FLAGS_NB,     ///< Total number of flags
} hexaworld_cell_flag_t;

/**
 * @brief Ways a hexagon can be drawn.
 */
typedef enum hexagon_draw_fillmode_t {
    DRAW_HEXAGON_FILL,
    DRAW_HEXAGON_LINES
} hexagon_draw_fillmode_t;

/// @brief a 16-width set of bit flags
typedef u16 flag_set16_t;

/// @brief a 8-width set of bit flags
typedef u8  flag_set8_t;

/// @brief temperature in celsius (human POV)
typedef i8 temp_c_t;

/// @brief sane altitude range for terrain
typedef i16 alt_m_t;

/// @brief freshwater height type
typedef u16 frwtr_m_t;

/// @brief ratio type (usually between 0.0f and 1.0f, but no guarantee)
typedef f32 ratio_t;

/**
 * @brief A single hexagonal cell.
 */
typedef struct hexa_cell_t {
    /// local angle of the tectonic plate 
    vector_2d_polar_t telluric_vector;
    /// mean wind direction and force
    vector_2d_polar_t winds_vector;

    /// freshwater direction on the tile
    cell_direction_t freshwater_direction;

    /// mean humidty on the tile
    ratio_t humidity;
    /// mean precipitations on the tile
    ratio_t precipitations;
    // vegetation coefficient
    ratio_t vegetation_cover;

    /// freshwater excess height
    frwtr_m_t freshwater_height;
    /// mean altitude of the tile
    alt_m_t altitude;
    /// unsigned integer containing the flags as bit offsets
    flag_set16_t flags;

    /// expected temperature of the tile
    temp_c_t temperature;
    /// bit flags representing wether a direction is considered as a freshwater source
    flag_set8_t freshwater_sources_directions;
} hexa_cell_t;

/**
 * @brief just the shape of an hexagon.
 */
typedef struct hexagon_shape_t {
    vector_2d_cartesian_t center;
    f32 radius;
} hexagon_shape_t;

/**
 * @brief Computes the position and radius of a hexagonal cell in a pixel rectangle.
 * 
 * @param[in] boundaries rectangle defined by the topleft coordinates and its sides' length (in pixels)
 * @param[in] x tile x-position in the world array
 * @param[in] y tile y-position in the world array
 * @param[in] width width, in tiles, of the world array
 * @param[in] height height, in tiles, of the world array
 * @return hexagon_shape_t pixel shape of the hexagon representing the cell's coordinates
 */
hexagon_shape_t hexagon_pixel_position_in_rectangle(f32 boundaries[4u], u32 x, u32 y, u32 width, u32 height);

/**
 * @brief Computes the hexagonal tile's position containing a certain point in a rectangle.
 * 
 * @param[in] boundaries rectangle defined by the topleft coordinates and its sides' length (in pixels)
 * @param[in] pix_x x-coordinates of a point in pixels
 * @param[in] pix_y y-coordinates of a point in pixels
 * @param[in] array_width width, in tiles, of the world array
 * @param[in] array_height height, in tiles, of the world array
 * @return vector_2d_cartesian_t integer coordinates of the tile if the point lies in the radius of an hexagon, or the array width and height if invalid
 */
vector_2d_cartesian_t hexagon_array_coords_from_rectangle(f32 boundaries[4u], u32 pix_x, u32 pix_y, u32 array_width, u32 array_height);

/**
 * @brief Draws a regular hexagon shape to the current raylib context.
 * 
 * @param[in] target_shape shape describing an hexagon
 * @param[in] color u32-coded color
 * @param[in] scale scale to be applied to the hexagon's radius
 * @param[in] fill wether to fill or not the hexagon
 */
void draw_hexagon(hexagon_shape_t *target_shape, u32 color, f32 scale, hexagon_draw_fillmode_t fill);

/**
 * @brief Determines the hexagonal cells pointed by an angle in radians going from a point in the center of those cell.
 * 
 * @param[in] cells_around pointer to cells around the given angle
 * @param[in] angle angle in radians
 * @param[out] out_pointed_cells outgoing pair of pointed cells by the angle. The first one is toward-antiradial cell, the second the toward-radial cell.
 * @param[out] out_pointed_cells_ratios outgoing ratio of "pointing" of the angle to the two cells.
 */
void hexa_cell_angle_pointed_cells(hexa_cell_t *cells_around[HEXAGON_SIDES_NB], f32 angle, hexa_cell_t *out_pointed_cells[2u], ratio_t out_pointed_cells_ratios[2u]);

/**
 * @brief Sets an bit flag in a cell.
 * 
 * @param[inout] cell target cell
 * @param[in] flag flag to set to 1
 */
void hexa_cell_set_flag(hexa_cell_t *cell, u32 flag);

/**
 * @brief Checks for the presence of a bit flag in  a cell.
 * 
 * @param[inout] cell target cell
 * @param[in] flag flag to check
 * @return 1 if the flag is set to 1, 0 otherwise
 */
u32 hexa_cell_has_flag(hexa_cell_t *cell, u32 flag);

#endif
