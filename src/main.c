/** @file main.c
 *
 *  N-GINE, a portable game engine which is being developed specifically
 *  for the Nokia N-Gage.
 *
 *  Minimal launcher implementation.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "ngine.h"

#if defined __SYMBIAN32__
#define RES_FILE "E:\\data.pfs"
#else
#define RES_FILE "data.pfs"
#endif

int main(int argc, char *argv[])
{
    int      status = 0;
    ngine_t *core   = NULL;

    status = ng_init(RES_FILE, "ngine", &core);
    if (NG_OK != status)
    {
        goto quit;
    }

    status = ng_load_map("entry.tmj", core);
    if (NG_OK != status)
    {
        goto quit;
    }

    while (NG_OK == status)
    {
        status = ng_update(core);
        if (NG_EXIT == status)
        {
            status = NG_OK;
            goto quit;
        }
    }

    quit:
    ng_unload_map(core);
    ng_free(core);
    return status;
}
