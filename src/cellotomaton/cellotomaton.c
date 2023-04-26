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
// ---- CONSTANTS ----------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

#define PENDULUM_ARRAY_PAIR_NB (2u)   ///< I actually fail to think of a use case where this number isn't 2.

// -------------------------------------------------------------------------------------------------
// ---- TYPE DEFINITIONS ---------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

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

/**
 * @brief Pointers to a cell's neighbors.
 */
typedef struct cell_neighbors_t {
    /// collection of pointers, one per direction
    void *neighbors[DIRECTIONS_NB];
} cell_neighbors_t;

/**
 * @brief A pendulum buffer is a buffer that can also hold data about its neighboring cells.
 * The neighboring cells are usually located in another pendulum buffer so the buffer's changes
 * are not reflected and would influence its current processing.
 */
typedef struct pendulum_buffer_t {
    /// buffer's data as a 2d array
    target_array_t data;
    /// map linking each cell's coordinates to its neighbors
    cell_neighbors_t **neighbors;
} pendulum_buffer_t;

/**
 * @brief Definition of a cell automaton data.
 */
typedef struct cell_automaton_t {
    /// companion foreign target array, its lifetime is unmanaged by the cell automaton's routines
    target_array_t target_array;
    /// two owned pendulum buffers to apply the automaton without any copy
    pendulum_buffer_t pendulum_buffers[PENDULUM_ARRAY_PAIR_NB];
} cell_automaton_t;

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DECLARATIONS --------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

/**
 * @brief Populates an array of anonymous pointers with the neighboring tiles of another defined by its coordinates.
 * 
 * @param[out] cell_neighs method-populated array
 * @param[in] x horizontal position of the center tile, in sub-array number
 * @param[in] y vertical position of the center tile, in number of bytes
 * @param[in] array all the tiles as an anonymous array
 */
static void get_neighbors(cell_neighbors_t *cell_neighs, size_t x, size_t y, target_array_t *array);

/**
 * @brief Copies the content and size properties of an array to another.
 * 
 * @param[out] dest copied-on array
 * @param[in] source copied-out array
 */
static void copy_array(target_array_t *dest, target_array_t *source);

/**
 * @brief Initializes an allocated pendulum buffer. This will allocate its internal data to the desired size.
 * 
 * @param[out] buffer target to-initialize buffer
 * @param[in] width width, in number of sub-arrays
 * @param[in] height height, in number of elements of size `stride`
 * @param[in] stride size of an element
 */
static void pendulum_buffer_initialize(pendulum_buffer_t *buffer, size_t width, size_t height, size_t stride);

/**
 * @brief Refreshes a buffer with some data
 * 
 * @param[inout] buffer initialized buffer
 * @param[in] source array containing the information. Must be of the same size, height and stride as the buffer.
 */
static void pendulum_buffer_refresh_from_array(pendulum_buffer_t *buffer, target_array_t source);

/**
 * @brief Link a pendulum buffer to another through its neighbors.
 * The buffer will have its cell's neighbors set to the alter ego's cells.
 * 
 * @param[inout] buffer initialized buffer
 * @param[in] alter_ego initialized buffer to target
 */
static void pendulum_buffer_link_to_alter_ego(pendulum_buffer_t *buffer, pendulum_buffer_t alter_ego);

/**
 * @brief Frees the memory allocated inside a buffer.
 * 
 * @param[inout] buffer initialized buffer
 */
static void pendulum_buffer_free(pendulum_buffer_t *buffer);

// -------------------------------------------------------------------------------------------------
// ---- HEADER FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
void otomaton_apply(cell_automaton_t *automaton, u32 iteration_nb, apply_to_cell_func_t function) {
    target_array_t *target_array = NULL;
    size_t active_buffer_index = 0u;

    // contengency
    if ((!automaton) || (!function) || !((automaton->target_array).tiles)) {
        return;
    }

    // shortening accesses
    target_array = &(automaton->target_array);

    // refreshing buffers to reflect the array's eventual exterior changes
    for (size_t i = 0 ; i < PENDULUM_ARRAY_PAIR_NB ; i++) {
        pendulum_buffer_refresh_from_array(automaton->pendulum_buffers + i, *target_array);
    }

    // applying the automaton function
    for (size_t i = 0u ; i < iteration_nb ; i++) {
        for (size_t x = 0u ; x < target_array->width ; x += 1u) {
            for (size_t y = 0u ; y < target_array->height ; y += 1u) {
                function(
                        automaton->pendulum_buffers[active_buffer_index].data.tiles[x] + y*(target_array->stride), 
                        automaton->pendulum_buffers[active_buffer_index].neighbors[x][y].neighbors);
            }
        }

        // alternating the buffers
        active_buffer_index = (active_buffer_index + 1u) % PENDULUM_ARRAY_PAIR_NB;
    }
    
    // everything went right (shock, gasp ?) so we commit the last buffer into the target array 
    copy_array(target_array, &(automaton->pendulum_buffers[(active_buffer_index + 1u) % PENDULUM_ARRAY_PAIR_NB].data));
}

// -------------------------------------------------------------------------------------------------
cell_automaton_t *otomaton_create(void **array, size_t width, size_t height, size_t stride) {
    cell_automaton_t *automaton = NULL;

    automaton = malloc(sizeof(*automaton));
    if (!automaton) {
        return NULL;
    }

    automaton->target_array = (target_array_t) { .tiles = array, .width = width, .height = height, .stride = stride };

    for (size_t i = 0u ; i < PENDULUM_ARRAY_PAIR_NB ; i++) {
        pendulum_buffer_initialize(automaton->pendulum_buffers + i, width, height, stride);
    }
    for (size_t i = 0u ; i < PENDULUM_ARRAY_PAIR_NB ; i++) {
        pendulum_buffer_link_to_alter_ego(automaton->pendulum_buffers + i, automaton->pendulum_buffers[(i + 1u) % PENDULUM_ARRAY_PAIR_NB]);
    }

    return automaton;
}

// -------------------------------------------------------------------------------------------------
void otomaton_destroy(cell_automaton_t **automaton) {
    for (size_t i = 0u ; i < 2u ; i++) {
        pendulum_buffer_free((*automaton)->pendulum_buffers + i);
    }

    free(*automaton);
    *automaton = NULL;
}

// -------------------------------------------------------------------------------------------------
// ---- STATIC FUNCTIONS DEFINITIONS ---------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void get_neighbors(cell_neighbors_t *cell_neighs, size_t x, size_t y, target_array_t *array) {
    size_t coord_w = (x - 1u) * (x > 0) + (array->width - 1u) * (x == 0);
    size_t coord_e = (x + 1u) % (array->width);

    size_t coord_n = (y - array->stride) * (y > 0) + (array->height*array->stride - array->stride) * (y == 0);
    size_t coord_s = (y + array->stride) % (array->height*array->stride);

    if ((y / array->stride) & 0x01) {
        // odd row
        cell_neighs->neighbors[DIRECTION_NW] = array->tiles[x]       + coord_n;
        cell_neighs->neighbors[DIRECTION_NE] = array->tiles[coord_e] + coord_n;
        cell_neighs->neighbors[DIRECTION_SW] = array->tiles[x]       + coord_s;
        cell_neighs->neighbors[DIRECTION_SE] = array->tiles[coord_e] + coord_s;
    } else {
        // even row
        cell_neighs->neighbors[DIRECTION_NW] = array->tiles[coord_w] + coord_n;
        cell_neighs->neighbors[DIRECTION_NE] = array->tiles[x]       + coord_n;
        cell_neighs->neighbors[DIRECTION_SW] = array->tiles[coord_w] + coord_s;
        cell_neighs->neighbors[DIRECTION_SE] = array->tiles[x]       + coord_s;
    }
    
    cell_neighs->neighbors[DIRECTION_E]  = array->tiles[coord_e] + y;
    cell_neighs->neighbors[DIRECTION_W]  = array->tiles[coord_w] + y;
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

// -------------------------------------------------------------------------------------------------
// ---- PENDULUM BUFFER FUNCTIONS  -----------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
static void pendulum_buffer_initialize(pendulum_buffer_t *buffer, size_t width, size_t height, size_t stride) {
    // allocating buffer space
    buffer->data.tiles = malloc(width * sizeof(*(buffer->data.tiles)));
    for (size_t i = 0u ; i < width ; i++) {
        buffer->data.tiles[i] = malloc(height * stride);
    }
    buffer->data.width = width;
    buffer->data.height = height;
    buffer->data.stride = stride;

    // allocating neighbors space
    buffer->neighbors = malloc(width * sizeof(*(buffer->neighbors)));
    for (size_t i = 0u ; i < width ; i++) {
        buffer->neighbors[i] = malloc(height * sizeof(*(buffer->neighbors[i])));
    }

    // the function does not return anything, so if a malloc returns null... what ?
}

// -------------------------------------------------------------------------------------------------
static void pendulum_buffer_link_to_alter_ego(pendulum_buffer_t *buffer, pendulum_buffer_t alter_ego) {
    for (size_t x = 0u ; x < buffer->data.width ; x++) {
        for (size_t y = 0u ; y < buffer->data.height ; y++) {
            get_neighbors(buffer->neighbors[x] + y, x, y*buffer->data.stride, &(alter_ego.data));
        }
    }
}

// -------------------------------------------------------------------------------------------------
static void pendulum_buffer_refresh_from_array(pendulum_buffer_t *buffer, target_array_t source) {
    copy_array(&(buffer->data), &source);
}

// -------------------------------------------------------------------------------------------------
static void pendulum_buffer_free(pendulum_buffer_t *buffer) {
    for (size_t i = 0u ; i < buffer->data.width ; i++) {
        free(buffer->neighbors[i]);
    }
    free(buffer->neighbors);

    for (size_t i = 0u ; i < buffer->data.width ; i++) {
        free(buffer->data.tiles[i]);
    }
    free(buffer->data.tiles);

    buffer->data.width = 0u;
    buffer->data.height = 0u;
    buffer->data.stride = 0u;
}