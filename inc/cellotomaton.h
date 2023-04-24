/**
 * @file cellotomaton.h
 * @author gabriel 
 * @brief Build, configure and destoy cellular automatons targeted to arrays containing hexagonal tiles.
 * @version 0.1
 * @date 2023-04-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __CELLOTOMATON_H__
#define __CELLOTOMATON_H__

#include <unstandard.h>

/**
 * @brief Possible directions from a hexagonal cell to another.
 * 
 */
typedef enum cell_direction_t {
    DIRECTION_NW,   ///< Top left cell
    DIRECTION_NE,   ///< Top right cell
    DIRECTION_W,    ///< Left cell
    DIRECTION_E,    ///< Right cell
    DIRECTION_SW,   ///< Bottom left cell
    DIRECTION_SE,   ///< Bottom right cell
    NB_DIRECTIONS,  ///< Number of possible directions
} cell_direction_t;

/**
 * @brief Sructure holding the data describing a cell automaton.
 * Declared as opaque, so its lifetime must be managed through the create() and destroy() functions.
 */
typedef struct cell_automaton_t cell_automaton_t;

/**
 * @brief Type of a function pointer accepted by the automaton.
 * @param[inout] target targeted cell which state will change.
 * @param[in] neighbors neighboring cells ordered through their direction.
 */
typedef void (*apply_to_cell_func_t)(void *target, void *neighbors[NB_DIRECTIONS]);

/**
 * @brief Applies the automaton on an anonymous bidimensional array. The array is modified by the operation.
 * If the automaton's function is NULL, nothing is done to the array.
 * 
 * @param[inout] automaton automaton to apply to the array, can be NULL (in this case, nothing will be done)
 * @param[in] iteration_nb number of times the function is applied to each cell
 * @param[inout] array a bidimensional array of some type representing an hesagonally-grided space
 * @param[in] width number of columns in the array (in numbers of sub-arrays)
 * @param[in] height number of rows in the array (in number of elements)
 * @param[in] stride size of the array's base type, in bytes
 */
void otomaton_apply(cell_automaton_t *automaton, u32 iteration_nb, void **array, size_t width, size_t height, size_t stride);

/**
 * @brief Creates an automaton on the heap and returns a pointer to it.
 * 
 * @param[in] func function stored in the automaton, and applied to an array.
 * @return cell_automaton_t* a pointer to the instance on the heap, is NULL if something went wrong
 */
cell_automaton_t *otomaton_create(apply_to_cell_func_t func);

/**
 * @brief Destroys an automaton and releases the resources taken by the instance. 
 * The function will set the pointed pointer to NULL.
 * 
 * @param[inout] automaton automaton to release
 */
void otomaton_destroy(cell_automaton_t **automaton);

/**
 * @brief Changes the applied function of the automaton.
 * 
 * @param[inout] automaton target automaton
 * @param[in] func new function (can be NULL)
 */
void otomaton_set_apply_function(cell_automaton_t *automaton, apply_to_cell_func_t func);

#endif
