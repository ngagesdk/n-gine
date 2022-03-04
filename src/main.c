// Spdx-License-Identifier: MIT

#include "core.h"

#if defined __SYMBIAN32__
#  define MAP_FILE_NAME     "E:\\overworld.tmj"
#  define TILESET_FILE_NAME "E:\\overworld.bmp"
#else
#  define MAP_FILE_NAME     "overworld.tmj"
#  define TILESET_FILE_NAME "overworld.bmp"
#endif

int main(int argc, char *argv[])
{
    int     status = 0;
    core_t *core   = NULL;

    status = init_core("demo", &core);
    if (CORE_OK != status)
    {
        goto quit;
    }

    status = load_map(MAP_FILE_NAME, TILESET_FILE_NAME, core);
    if (CORE_OK != status)
    {
        goto quit;
    }

    while (CORE_OK == status)
    {
        status = update_core(core);
        if (CORE_EXIT == status)
        {
            status = CORE_OK;
            goto quit;
        }
    }

    quit:
    unload_map(core);
    free_core(core);
    return status;
}
