
#ifndef __HEXAWORLDCOMPONENTS_H__
#define __HEXAWORLDCOMPONENTS_H__

#include <unstandard.h>
#include <cellotomaton.h>

#include "layers.h"

// -------------------------------------------------------------------------------------------------
// ---- CONSTANTS ----------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

#define HEXAGON_SIDES_NB (6u)     ///< number of sides of an hexagon. tough.
#define PI_T_2 ((2.0f) * (PI))    ///< 2 times pi

#define TELLURIC_VECTOR_SEEDING_INV_CHANCE (0x40)  ///< the greater, the bigger the chance a telluric tile is NOT seeded.
#define TELLURIC_VECTOR_DIRECTIONS_NB (32)     ///< number of possible directions for a telluric vector
#define TELLURIC_VECTOR_UNIT_ANGLE ((PI_T_2) / (TELLURIC_VECTOR_DIRECTIONS_NB))      ///< telluric vector minimum angle 

#define LANDMASS_SEEDING_CHANCE (0x03)    ///< the greater, the bigger the chance a land tile is seeded.

#define ALTITUDE_MAX (3000)  ///< maximum altitude, in meters
#define ALTITUDE_MIN (-2000)  ///< minimum altitude, in meters

#define WINDS_VECTOR_DIRECTIONS_NB (32)
#define WINDS_VECTOR_UNIT_ANGLE ((PI_T_2) / (WINDS_VECTOR_DIRECTIONS_NB))      ///< winds vector minimum angle 

#define FRESHWATER_PRECIPITATIONS_MULTIPLIER (1.5f)   ///< freshwater source generation where there is precipitations
#define FRESHWATER_MOUNTAIN_NO_SOURCE_CHANCE (0x10)    ///< chance that a source is generated on a mountain

// -------------------------------------------------------------------------------------------------
// ---- TYPEDEFS -----------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Describes wether a cell automaton should iterates an absolute number of times or a number of times relative to the array size.
 */
typedef enum layer_gen_iteration_type_t {
    LAYER_GEN_ITERATE_ABSOLUTE,
    LAYER_GEN_ITERATE_RELATIVE,
} layer_gen_iteration_type_t;

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

    HEXAW_FLAGS_NB,     ///< Total number of flags
} hexaworld_cell_flag_t;

/**
 * @brief A single hexagonal cell.
 */
typedef struct hexa_cell_t {
    /// long unsigned integer containing the flags as bit offsets
    u64 flags;
    
    /// local angle of the tectonic plate 
    vector_2d_polar_t telluric_vector;
    /// mean altitude of the tile
    i32 altitude;
    /// mean wind direction and force
    vector_2d_polar_t winds_vector;
    /// mean humidty on the tile
    f32 humidity;
    /// mean precipitations on the tile
    f32 precipitations;
    /// freshwater excess height on the tile
    u32 freshwater_height;
} hexa_cell_t;

/**
 * @brief just the shape of an hexagon.
 */
typedef struct hexagon_shape_t {
    vector_2d_cartesian_t center;
    f32 radius;
} hexagon_shape_t;

/**
 * @brief Function pointer as the prototype of some code handling the drawing of a single cell.
 */
typedef void (*layer_draw_function_t)(hexa_cell_t *cell, hexagon_shape_t *target_shape);

// forward declaration
struct hexaworld_t;
/**
 * @brief Function pointer as the prototype of some code handling the seeding of some layer.
 */
typedef void (*layer_seed_function_t)(struct hexaworld_t *world);

/**
 * @brief Aggregation of all the functions working on a single layer to create it and display it.
 */
typedef struct layer_calls_t {
    /// function drawing a layer's single cell
    layer_draw_function_t draw_func;
    /// function seeding the whole world before the automaton
    layer_seed_function_t seed_func;
    /// function applied by the automaton to generate a single cell
    apply_to_cell_func_t automaton_func;
    /// function applied by the automaton to create the flags of a single cell
    apply_to_cell_func_t flag_gen_func;
    /// number of times the automaton applies the `automaton_func` toeach cell of the world
    u32 automaton_iter;
    /// way the automaton should iterate over the array
    layer_gen_iteration_type_t iteration_flavour;
} layer_calls_t;

// -------------------------------------------------------------------------------------------------
typedef struct hexaworld_t { 
    /// 2d heap-allocated array of the tiles
    hexa_cell_t **tiles;
    /// number of tiles on the x-axis
    size_t width;
    /// number of tiles on the y-axis
    size_t height;

    /// pointer to an heap-allocated cellular automaton for layer generation
    cell_automaton_t *automaton;

    /// layers generation functions
    layer_calls_t hexaworld_layers_functions[HEXAW_LAYERS_NUMBER];
} hexaworld_t;

// -------------------------------------------------------------------------------------------------
// ---- INTERFACE FUNCTIONS ------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

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
hexagon_shape_t hexagon_position_in_rectangle(f32 boundaries[4u], u32 x, u32 y, u32 width, u32 height);

// -------------------------------------------------------------------------------------------------
// ---- LAYERS CALLS DATA --------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

extern const layer_calls_t telluric_layer_calls;
extern const layer_calls_t landmass_layer_calls;
extern const layer_calls_t altitude_layer_calls;
extern const layer_calls_t winds_layer_calls;
extern const layer_calls_t humidity_layer_calls;
extern const layer_calls_t freshwater_layer_calls;

#endif
