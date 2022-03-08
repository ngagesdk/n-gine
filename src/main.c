// Spdx-License-Identifier: MIT

#include "core.h"

int main(int argc, char *argv[])
{
    int     status = 0;
    core_t *core   = NULL;

    status = init_core("nrpg", &core);
    if (CORE_OK != status)
    {
        goto quit;
    }

    status = load_map("lowtown.tmj", core);
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
