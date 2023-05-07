
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <endoftheline.h>
#include <hexaworld_application.h>
#include <unstandard.h>

static void intHandler(int val) {
    end_of_the_line(END_OF_THE_LINE_EXIT_INTERRUPTED, "interrupted by signal.");
}

i32 main(u32 argc, char const *argv[]) {
    hexaworld_raylib_app_handle_t *application = NULL;

    signal(SIGINT, &intHandler);

    u32 index_args = 1u;
    i32 seed = 0;
    u32 width = 20u;
    u32 height = 20u;

    // fetching command-line args
    while (index_args < argc) {
        if ((strcmp(argv[index_args], "-s") == 0) && ((index_args + 1u) < argc)) {
            index_args += 1u;
            seed = strtoul(argv[index_args], NULL, 0);
        } else if ((strcmp(argv[index_args], "-x") == 0) && ((index_args + 1u) < argc)) {
            index_args += 1u;
            width = strtoul(argv[index_args], NULL, 0);
        } else if ((strcmp(argv[index_args], "-y") == 0) && ((index_args + 1u) < argc)) {
            index_args += 1u;
            height = strtoul(argv[index_args], NULL, 0);
        } else {
            end_of_the_line(END_OF_THE_LINE_EXIT_INVALID_ARGS, "\n\tusage :\n\t$ otomaton [-s seed] [-x width] [-y height]\n");
            return -1;
        }
        index_args += 1u;
    }

    // creating application
    application = hexaworld_raylib_app_init(seed, 1200u, 800u, width, height);

    // running the application
    hexaworld_raylib_app_run(application, 20u);

    // deallocating
    hexaworld_raylib_app_deinit(&application);

    return 0;
}
