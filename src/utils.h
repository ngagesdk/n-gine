// Spdx-License-Identifier: MIT

#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>
#include "ngine.h"

typedef struct aabb
{
    Uint8 bottom;
    Uint8 left;
    Uint8 right;
    Uint8 top;

} aabb_t;

SDL_bool bb_do_intersect(const aabb_t bb_a, const aabb_t bb_b);
Uint64   generate_hash(const unsigned char* name);
status_t load_texture_from_file(const char* file_name, SDL_Texture** texture, ngine_t* core);
status_t set_display_text(const char* text, ngine_t* core);
void     clear_display_text(ngine_t* core);
void     render_text(ngine_t* core);

#endif /* UTILS_H */
