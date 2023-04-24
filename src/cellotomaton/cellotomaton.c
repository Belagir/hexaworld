/**
 * @file cellotomaton.c
 * @author gabriel 
 * @brief Definition file for the cell automaton.
 * @version 0.1
 * @date 2023-04-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <stdlib.h>
#include <cellotomaton.h>

// -------------------------------------------------------------------------------------------------
// ---- TYPE DEFINITIONS ---------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Definition of a cell automaton data.
 */
typedef struct cell_automaton_t {
    /// applied function : the function will receive one cell's neighbors and change the state of this cell.
    apply_to_cell_func_t func;
} cell_automaton_t;

/**
 * @brief Data about an array handled by the automaton.
 */
typedef struct target_array_t {
    /// anonymous array of pointers to the array's data
    void **tiles;
    /// number of columns, in number of sub arrays
    size_t width;
    /// number of rows, in number of elements
    size_t height;
    /// size of the base type, in bytes
    size_t stride;
} target_array_t;

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DECLARATIONS --------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Populates an array of anonymous pointers with the neighboring tiles of another defined by its coordinates.
 * 
 * @param[out] neighbors_tiles populated array
 * @param[in] x horizontal position of the center tile
 * @param[in] y vertical position of the center tile
 * @param[in] array all the tiles
 */
static void get_neighbors(void *neighbors_tiles[NB_DIRECTIONS], size_t x, size_t y, target_array_t *array);

/**
 * @brief Copies the content and size properties of an array to another.
 * 
 * @param[out] dest copied-on array
 * @param[in] source copied-out array
 */
static void copy_array(target_array_t *dest, target_array_t *source);

// -------------------------------------------------------------------------------------------------
// ---- HEADER FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
void otomaton_apply(cell_automaton_t *automaton, u32 iteration_nb, void **array, size_t width, size_t height, size_t stride) {
    void *neighbors_tiles[NB_DIRECTIONS] = { 0u };
    target_array_t target_array = { 0u }; 
    target_array_t buffered_array = { 0u };

    // contengency
    if ((!automaton) || !(automaton->func)) {
        return;
    }
    
    // creating a wrapper for our inputed array
    target_array = (target_array_t) { .tiles = array, .width = width, .height = height, .stride = stride };

    // allocating our buffer
    buffered_array.tiles = malloc(width * sizeof(*buffered_array.tiles));
    for (size_t i = 0 ; i < width ; i++) {
        buffered_array.tiles[i] = malloc(height * stride);
    }

    // running the automaton on the buffer, the copying the buffer to the original array...
    for (size_t i = 0u ; i < iteration_nb ; i++) {
        copy_array(&buffered_array, &target_array);
        for (size_t x = 0u ; x < width ; x += 1u) {
            for (size_t y = 0u ; y < height*stride ; y += stride) {
                get_neighbors(neighbors_tiles, x, y, &buffered_array);
                automaton->func(target_array.tiles[x] + y, neighbors_tiles);
            }
        }
    }

    // freeing our buffer
    for (size_t i = 0 ; i < width ; i++) {
        free(buffered_array.tiles[i]);
    }
    free(buffered_array.tiles);
}

// -------------------------------------------------------------------------------------------------
cell_automaton_t *otomaton_create(apply_to_cell_func_t func) {
    cell_automaton_t *automaton = NULL;

    automaton = malloc(sizeof(*automaton));
    otomaton_set_apply_function(automaton, func);

    return automaton;
}

// -------------------------------------------------------------------------------------------------
void otomaton_destroy(cell_automaton_t **automaton) {
    free(*automaton);
    *automaton = NULL;
}

// -------------------------------------------------------------------------------------------------
void otomaton_set_apply_function(cell_automaton_t *automaton, apply_to_cell_func_t func) {
    if (automaton) {
        automaton->func = func;
    }
}

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void get_neighbors(void *neighbors_tiles[NB_DIRECTIONS], size_t x, size_t y, target_array_t *array) {
    size_t coord_w = (x - 1u) * (x > 0) + (array->width - 1u) * (x == 0);
    size_t coord_e = (x + 1u) % (array->width);

    size_t coord_n = (y - array->stride) * (y > 0) + (array->height*array->stride - array->stride) * (y == 0);
    size_t coord_s = (y + array->stride) % (array->height*array->stride);

    if ((y / array->stride) & 0x01) {
        // odd row
        neighbors_tiles[DIRECTION_NW] = array->tiles[x]       + coord_n;
        neighbors_tiles[DIRECTION_NE] = array->tiles[coord_e] + coord_n;
        neighbors_tiles[DIRECTION_SW] = array->tiles[x]       + coord_s;
        neighbors_tiles[DIRECTION_SE] = array->tiles[coord_e] + coord_s;
    } else {
        // even row
        neighbors_tiles[DIRECTION_NW] = array->tiles[coord_w] + coord_n;
        neighbors_tiles[DIRECTION_NE] = array->tiles[x]       + coord_n;
        neighbors_tiles[DIRECTION_SW] = array->tiles[coord_w] + coord_s;
        neighbors_tiles[DIRECTION_SE] = array->tiles[x]       + coord_s;
    }
    
    neighbors_tiles[DIRECTION_E]  = array->tiles[coord_e] + y;
    neighbors_tiles[DIRECTION_W]  = array->tiles[coord_w] + y;
}

// -------------------------------------------------------------------------------------------------
static void copy_array(target_array_t *dest, target_array_t *source) {
    dest->width = source->width;
    dest->height = source->height;
    dest->stride = source->stride;

    for (size_t i = 0 ; i < source->width ; i++) {
        bytewise_copy(dest->tiles[i], source->tiles[i], source->height*source->stride);
    }
}
