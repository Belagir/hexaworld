
#include <endoftheline.h>

#include <stdlib.h>
#include <stdio.h>

#include <unstandard.h>

#define END_OF_THE_LINE_MAX_CALLS 16u   ///< MAximum number of calls stored

/**
 * @brief Describes a call made at the end of the program
 * "One last thing..."
 */
typedef struct last_thing_done_t {
    ender_function_t call_f;
    void *arg;
} last_thing_done_t;

/**
 * @brief monitoring wether the end of the line was triggered while end-of-the-lining
 */
static volatile i32 shared_end_of_the_line_flag = 0;

/**
 * @brief module data
 */
static struct {
    last_thing_done_t last_things[END_OF_THE_LINE_MAX_CALLS];
    size_t nb_last_things;
} module_data = { 0u };

/**
 * @brief Messages displayed for each exit code
 */
static const char *end_code_to_string[END_OF_THE_LINE_EXIT_CODES_NB] = {
        //END_OF_THE_LINE_EXIT_INVALID_ARGS
        "INVALID ARGUMENTS PASSED TO THE COMMAND LINE",
        // END_OF_THE_LINE_EXIT_RAYLIB_FAILED
        "RAYLIB FAILED TO LAUNCH WINDOW",
        // END_OF_THE_LINE_EXIT_NO_MEMORY
        "PROGRAM COULDN'T ALLOCATE MEMORY",
        // END_OF_THE_LINE_EXIT_INTERRUPTED
        "PROGRAM WAS INTERRUPTED",
        // END_OF_THE_LINE_EXIT_DOUBLE_INTERRUPTED
        "PROGRAM WAS INTERRUPTED TWICE"
};

// -------------------------------------------------------------------------------------------------
void end_of_the_line_register_call(ender_function_t call_f, void *arg) {
    if (module_data.nb_last_things >= END_OF_THE_LINE_MAX_CALLS) {
        return;
    }

    module_data.last_things[module_data.nb_last_things].call_f = call_f;
    module_data.last_things[module_data.nb_last_things].arg = arg;

    module_data.nb_last_things += 1u;
}

// -------------------------------------------------------------------------------------------------
void end_of_the_line(end_of_the_line_exit_code_t code, char *msg) {
    ender_function_t called_function = NULL;
    void *args_given = NULL;

    if (shared_end_of_the_line_flag != 0) {
        if (code == END_OF_THE_LINE_EXIT_INTERRUPTED) {
            code = END_OF_THE_LINE_EXIT_DOUBLE_INTERRUPTED;
        }
    } else {
        shared_end_of_the_line_flag = 1;

        while (module_data.nb_last_things--) {
            called_function = module_data.last_things[module_data.nb_last_things].call_f;
            args_given = module_data.last_things[module_data.nb_last_things].arg;

            if (called_function) {
                called_function(args_given);
            }
        }
    }

    printf("[END OF THE LINE] : %s\nMessage : %s\n", end_code_to_string[code], msg);
    exit(code);
}
