/**
 * @file hexaworldcomponents.h
 * @author gabriel 
 * @brief Declaration of components (data structures, constants, layer descriptors, and some helper methods) needed to build a world.
 * @version 0.1
 * @date 2023-04-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __HEXAWORLDCOMPONENTS_H__
#define __HEXAWORLDCOMPONENTS_H__

#include <unstandard.h>
#include <cellotomaton.h>
#include <hexagonparadigm.h>

#include "layers.h"

// -------------------------------------------------------------------------------------------------
// ---- CONSTANTS ----------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

#define TELLURIC_SEED_NB_MIN (3u)
#define TELLURIC_SEED_NB_PER_TILE (0.005f)
#define TELLURIC_VECTOR_DIRECTIONS_NB (32)     ///< number of possible directions for a telluric vector
#define TELLURIC_VECTOR_UNIT_ANGLE ((PI_T_2) / (TELLURIC_VECTOR_DIRECTIONS_NB))      ///< telluric vector minimum angle 

#define LANDMASS_SEEDING_CHANCE (0x03)    ///< the greater, the bigger the chance a land tile is seeded.
#define LANDMASS_NO_ISLE_CHANCE (0x03)    ///<  the greater, the smaller the chance a sile flag is created.

#define ALTITUDE_MAX (4000)  ///< maximum altitude, in meters
#define ALTITUDE_MIN (-3000)  ///< minimum altitude, in meters
#define ALTITUDE_EROSION_INERTIA_WEIGHT (100) ///< inertia of the eroded cell
#define ALTITUDE_EROSION_RAND_VARIATION (30)    ///< random variation of altitude

#define TEMPERATURE_MAX (30)    ///< maximum temperature (on the equator at sea level)
#define TEMPERATURE_MIN (-25)   ///< minimum temperature (on the poles at sea level)
#define TEMPERATURE_RANGE (TEMPERATURE_MAX - TEMPERATURE_MIN)   ///< total temperature range
#define TEMPERATURE_ALTITUDE_MULTIPLIER (-0.00625f)     ///< °C lost with every meter of altitude
#define TEMPERATURE_RANDOM_SHIFT (0.3f)
#define TEMPERATURE_VARIANCE_LATITUDE (0.3f)

#define WINDS_VECTOR_DIRECTIONS_NB (32)     ///< number of possible direction for a wind vector 
#define WINDS_VECTOR_UNIT_ANGLE ((PI_T_2) / (WINDS_VECTOR_DIRECTIONS_NB))      ///< winds vector minimum angle 

#define CLOUD_COVER_DIFFUSION (1.2f)

#define FRESHWATER_PRECIPITATIONS_THRESHOLD (0.15f)      ///< threshold from which a precipitation on a tiles creates a source of water.
#define FRESHWATER_SOURCE_START_DEPTH (1u)      ///< start value for freshwater
#define FRESHWATER_MOUNTAIN_NO_SOURCE_CHANCE (0x08)    ///< chance that a source is generated on a mountain
#define FRESHWATER_WATERFALL_HEIGHT_THRESHOLD (2000)    ///< height in meters between two tiles needed for a waterfall to form
#define FRESHWATER_LAKE_DEPTH_THRESHOLD (5u)        ///< depth from which a body of water is considered a lake

#define VEGETATION_TEMPERATURE_MEAN (18)    ///< ideal temperature for things to grow
#define VEGETATION_TEMPERATURE_VARI (12)    ///< squared variation in temperature tolerated by plants
#define VEGETATION_CUTOUT_THRESHOLD (0.01f) ///< threshold from which the vegetation cover is ignored by some methods
#define VEGETATION_COVER_DIFFUSION_FACTOR (0.85f)   ///< dictates how much neighboring grassy tiles influence a tile to be more grassy even without wwater
#define VEGETATION_COVER_HELP_FOR_TREES (0.85f)     ///< how grass and small vegetation helps trees to grow
#define VEGETATION_TREES_PROPAGATION_WEIGHT (17.0f) ///< the greater, the clearer the cutout will be between forests (good results from 10.0f)
#define VEGETATION_TREES_PROPAGATION_OFFSET (0.35f) ///< offseting the transfer function from a forested tile to an empty one, the greater the harder it will be for a forested tile to expand

#define WHOLE_WORLD_OCEAN_ABYSS_CUTOUT (0.50f)  ///< height ratio for abyss ocean -> normal ocean drawing 
#define WHOLE_WORLD_OCEAN_REEF_CUTOUT  (0.25f)  ///< height ratio for normal ocean -> reef ocean drawing

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
    /// layers generation functions
    layer_calls_t hexaworld_layers_functions[HEXAW_LAYERS_NUMBER];

    /// 2d heap-allocated array of the tiles
    hexa_cell_t **tiles;
    /// number of tiles on the x-axis
    size_t width;
    /// number of tiles on the y-axis
    size_t height;

    /// pointer to an heap-allocated cellular automaton for layer generation
    cell_automaton_t *automaton;

    /// seed used for the map generation
    i32 map_seed;
} hexaworld_t;

// -------------------------------------------------------------------------------------------------
// ---- LAYERS CALLS DATA --------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

extern const layer_calls_t telluric_layer_calls;

extern const layer_calls_t landmass_layer_calls;

extern const layer_calls_t altitude_layer_calls;

extern const layer_calls_t winds_layer_calls;

extern const layer_calls_t cloud_cover_layer_calls;

extern const layer_calls_t freshwater_layer_calls;

extern const layer_calls_t vegetation_layer_calls;

extern const layer_calls_t temperature_layer_calls;

extern const layer_calls_t whole_world_layer_calls;

#endif
