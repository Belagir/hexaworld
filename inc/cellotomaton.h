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
#include <hexatypes.h>

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
typedef void (*apply_to_cell_func_t)(void *target, void *neighbors[DIRECTIONS_NB]);

/**
 * @brief Applies the automaton on its anonymous bidimensional array. The array is modified by the operation.
 * If the automaton's function is NULL, nothing is done to the array.
 * 
 * @param[inout] automaton automaton to apply to the array, can be NULL (in this case, nothing will be done)
 * @param[in] iteration_nb number of times the function is applied to each cell
 * @param[in] function function to apply to each cell
 */
void otomaton_apply(cell_automaton_t *automaton, u32 iteration_nb, apply_to_cell_func_t function);

/**
 * @brief Creates an automaton on the heap and returns a pointer to it.
 * 
 * @param[in] array the array on which every operation will be applied
 * @param[in] width width, in number of sub-arrays, of the main array
 * @param[in] height height, in number of elements of a sub-array
 * @param[in] stride size in bytes of an element of a sub-array
 * @return cell_automaton_t* a pointer to the instance on the heap, is NULL if something went wrong
 */
cell_automaton_t *otomaton_create(void **array, size_t width, size_t height, size_t stride);

/**
 * @brief Destroys an automaton and releases the resources taken by the instance. 
 * The function will set the pointed pointer to NULL.
 * 
 * @param[inout] automaton automaton to release
 */
void otomaton_destroy(cell_automaton_t **automaton);

#endif
