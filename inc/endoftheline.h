/**
 * @file endoftheline.h
 * @author gabriel 
 * @brief This is the end of the line, the Omega of the Alpha. The infinite entropy that comes at the program's end.
 * This module is tasked with terminating the program after it reachs an invalid and unrecoverable state after an interaction with the outer world, like the user or their system. The call to `end_of_the_line` is supposed to release all registered resources.
 * @version 0.1
 * @date 2023-05-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __ENDOFTHELINE_H__
#define __ENDOFTHELINE_H__

/**
 * @brief Kind of code the module can exit with.
 */
typedef enum end_of_the_line_exit_code_t {
    END_OF_THE_LINE_EXIT_INVALID_ARGS,
    END_OF_THE_LINE_EXIT_RAYLIB_FAILED,
    END_OF_THE_LINE_EXIT_NO_MEMORY,
    END_OF_THE_LINE_EXIT_INTERRUPTED,
    END_OF_THE_LINE_EXIT_DOUBLE_INTERRUPTED,

    END_OF_THE_LINE_EXIT_CODES_NB,
} end_of_the_line_exit_code_t;

/**
 * @brief Flavour of function needed by the module to register a call taht will be executed when a crash occurs.
 */
typedef void (*ender_function_t)(void *);

/**
 * @brief Registers a call that will be executed right as the program is tasked with terminating itself through `end_of_the_line`.
 * 
 * @param call_f pointer to the function to call
 * @param arg pointer to whatever the function needs
 */
void end_of_the_line_register_call(ender_function_t call_f, void * arg);

/**
 * @brief Ends the program, call each previously given function in the reverse order they were registered, prints a message to stdout, and exits with a certain code.
 * 
 * @param code broad reason for the crash
 * @param msg message to print to stdout
 */
void end_of_the_line(end_of_the_line_exit_code_t code, char *msg);

#endif
