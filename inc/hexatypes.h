
#ifndef __HEXATYPES_H__
#define __HEXATYPES_H__

/**
 * @brief Possible directions from a hexagonal cell to another. Ordered along the unit circle.
 * 
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

#endif
