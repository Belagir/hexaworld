
#ifndef __LAYERS_H__
#define __LAYERS_H__

/**
 * @brief List of the world's unique layers.
 */
typedef enum hexaworld_layer_t {
    HEXAW_LAYER_TELLURIC,   ///< Layer representing the movement of the tectonic plates.
    HEXAW_LAYER_LANDMASS,   ///< Layer representing the raw landmasses without elevation.
    HEXAW_LAYER_ALTITUDE,   ///< Layer representing the different altitudes of the world.
    HEXAW_LAYER_WINDS,      ///< Layer representing the mean wind direction and force over the land.
    HEXAW_LAYER_HUMIDITY,   ///< Layer representing the humidity level of a tile.
    HEXAW_LAYER_FRESHWATER, ///< Layer representing the rivers and lakes of a tile.

    HEXAW_LAYERS_NUMBER,    ///< Total number of layers.
} hexaworld_layer_t;

#endif
