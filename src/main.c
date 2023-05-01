
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hexaworld_application.h>
#include <unstandard.h>

i32 main(u32 argc, char const *argv[]) {
    hexaworld_raylib_app_handle_t *application = NULL;

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
            printf("usage : \n$ %s [-s seed] [-x width] [-y height]\n", argv[0]);
            return -1;
        }
        index_args += 1u;
    }

    // creating application
    application = hexaworld_raylib_app_init(seed, 1200u, 800u, width, height);
    if (!application) {
        return -1;
    }

    // running the application
    hexaworld_raylib_app_run(application, 20u);

    // deallocating
    hexaworld_raylib_app_deinit(&application);

    return 0;
}
