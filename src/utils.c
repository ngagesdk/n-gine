// SPDX-License-Identifier: MIT

#include <SDL.h>
#include "utils.h"
#include "types.h"

SDL_bool bb_do_intersect(const aabb_t bb_a, const aabb_t bb_b)
{
    Sint32 bb_a_x = bb_b.left - bb_a.right;
    Sint32 bb_a_y = bb_b.top  - bb_a.bottom;
    Sint32 bb_b_x = bb_a.left - bb_b.right;
    Sint32 bb_b_y = bb_a.top  - bb_b.bottom;

    if (0 < bb_a_x || 0 < bb_a_y)
    {
        return SDL_FALSE;
    }

    if (0 < bb_b_x || 0 < bb_b_y)
    {
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

/* djb2 by Dan Bernstein
 * http://www.cse.yorku.ca/~oz/hash.html
 */
Uint64 generate_hash(const unsigned char* name)
{
    Uint64 hash = 5381;
    Uint32 c;

    while ((c = *name++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

status_t load_texture_from_file(const char* file_name, SDL_Texture** texture, core_t* core)
{
    Uint8*       resource_buf;
    SDL_RWops*   resource;
    SDL_Surface* surface;

    if (! file_name)
    {
        return CORE_WARNING;
    }

    resource_buf = (Uint8*)load_binary_file_from_path(file_name);
    if (! resource_buf)
    {
        // SDL_Log("Failed to load resource: %s", file_name);
        return CORE_ERROR;
    }

    resource = SDL_RWFromConstMem((Uint8*)resource_buf, size_of_file(file_name));
    if (! resource)
    {
        free(resource_buf);
        // SDL_Log("Failed to convert resource %s: %s", file_name, SDL_GetError());
        return CORE_ERROR;
    }

    surface = SDL_LoadBMP_RW(resource, SDL_TRUE);
    if (! surface)
    {
        free(resource_buf);
        // SDL_Log("Failed to load image: %s", SDL_GetError());
        return CORE_ERROR;
    }
    free(resource_buf);

    if (0 != SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0xff, 0x00, 0xff)))
    {
        // SDL_Log("Failed to set color key for %s: %s", file_name, SDL_GetError());
    }
    if (0 != SDL_SetSurfaceRLE(surface, 1))
    {
        // SDL_Log("Could not enable RLE for surface %s: %s", file_name, SDL_GetError());
    }

    *texture = SDL_CreateTextureFromSurface(core->renderer, surface);
    if (! *texture)
    {
        // SDL_Log("Could not create texture from surface: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return CORE_ERROR;
    }
    SDL_FreeSurface(surface);

    // SDL_Log("Loading image from file: %s.", file_name);

    return CORE_OK;
}
