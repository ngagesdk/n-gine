// Spdx-License-Identifier: MIT

#include "core.h"

#if defined __SYMBIAN32__
#define RES_FILE "E:\\data.pfs"
#else
#define RES_FILE "data.pfs"
#endif

int main(int argc, char *argv[])
{
    int     status = 0;
    core_t *core   = NULL;

    status = init_core(RES_FILE, "nrpg", &core);
    if (CORE_OK != status)
    {
        goto quit;
    }

    status = load_map("1.tmj", core);
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
