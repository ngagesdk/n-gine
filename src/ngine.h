/** @file ngine.h
 *
 *  N-GINE, a portable game engine which is being developed specifically
 *  for the Nokia N-Gage.
 *
 *  API definition.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef NGINE_H
#define NGINE_H

#include "ngtypes.h"

#ifndef FUNCTION_NAME
#define FUNCTION_NAME ""
#endif

status_t ng_init_core(const char* resource_file, const char* title, ngine_t** core);
status_t ng_update_core(ngine_t* core);
void     ng_free_core(ngine_t *core);
status_t ng_load_map(const char* map_name, ngine_t* core);
void     ng_unload_map(ngine_t* core);

#endif /* NGINE_H */
