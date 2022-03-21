/** @file utils.c
 *
 *  N-GINE, a portable game engine which is being developed specifically
 *  for the Nokia N-Gage.
 *
 *  Utility functions.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL.h>
#include <stb_sprintf.h>
#include "ngine.h"

static void get_character_position(const unsigned char character, int* pos_x, int* pos_y)
{
    Sint32 index = 0;

    // If the character is not valid, select space.
    if ((character < 0x20) || (character > 0x7e))
    {
        index = 0;
    }
    else
    {
        index = character - 0x20;
    }

    *pos_x = (index % 18) * 7;
    *pos_y = (index / 18) * 9;
}

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

status_t load_texture_from_file(const char* file_name, SDL_Texture** texture, ngine_t* core)
{
    Uint8*       resource_buf;
    SDL_RWops*   resource;
    SDL_Surface* surface;

    if (! file_name)
    {
        return NG_WARNING;
    }

    resource_buf = (Uint8*)load_binary_file_from_path(file_name);
    if (! resource_buf)
    {
        // SDL_Log("Failed to load resource: %s", file_name);
        return NG_ERROR;
    }

    resource = SDL_RWFromConstMem((Uint8*)resource_buf, size_of_file(file_name));
    if (! resource)
    {
        free(resource_buf);
        // SDL_Log("Failed to convert resource %s: %s", file_name, SDL_GetError());
        return NG_ERROR;
    }

    surface = SDL_LoadBMP_RW(resource, SDL_TRUE);
    if (! surface)
    {
        free(resource_buf);
        // SDL_Log("Failed to load image: %s", SDL_GetError());
        return NG_ERROR;
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
        return NG_ERROR;
    }
    SDL_FreeSurface(surface);

    // SDL_Log("Loading image from file: %s.", file_name);

    return NG_OK;
}

status_t set_display_text(const char* text, ngine_t* core)
{
    size_t text_length = strlen(text);

    if (text_length > 144)
    {
        text_length = 144;
    }

    core->display_text = (unsigned char*)calloc(text_length + 1, sizeof(unsigned char));
    if (! core->display_text)
    {
        // SDL_Log("%s: error allocating memory.", FUNCTION_NAME);
        return NG_ERROR;
    }

    stbsp_snprintf(core->display_text, text_length + 1, "%s", text);

    return NG_OK;
}

void clear_display_text(ngine_t* core)
{
    core->display_text = NULL;
}

// Error handling: yes or no?
void render_text(ngine_t* core)
{
    SDL_Rect textbox      = { 0, 144, 176, 64 };
    SDL_Rect border_a     = { 0, 144, 176, 64 };
    SDL_Rect border_b     = { 2, 146, 172, 60 };
    SDL_Rect src          = { 0,   0,   7,  9 };
    SDL_Rect dst          = { 4, 149,   7,  9 };
    int      string_index = 0;
    int      col, row;

    SDL_SetRenderDrawColor(core->renderer, 0xff, 0xff, 0xff, 0x00);
    SDL_RenderFillRect(core->renderer, &textbox);
    SDL_RenderDrawRect(core->renderer, &textbox);
    SDL_SetRenderDrawColor(core->renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderDrawRect(core->renderer, &border_a);
    SDL_RenderDrawRect(core->renderer, &border_b);

    for (row = 0; row < 6; row += 1)
    {
        dst.x = 4;
        for (col = 0; col < 24; col += 1)
        {
            if ('\0' == core->display_text[string_index])
            {
                goto no_text_left;
            }
            get_character_position(core->display_text[string_index], &src.x, &src.y);

            if (' ' == core->display_text[string_index] && (4 == dst.x))
            {
                dst.x -= 7;
                col   -= 1;
            }
            string_index += 1;

            SDL_RenderCopy(core->renderer, core->font_texture, &src, &dst);
            dst.x += 7;
        }
        dst.y += 9;
    }
no_text_left:
}
