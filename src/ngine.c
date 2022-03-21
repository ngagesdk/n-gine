/** @file ngine.c
 *
 *  N-GINE, a portable game engine which is being developed specifically
 *  for the Nokia N-Gage.
 *
 *  API implementation.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL.h>
#include "ngine.h"

status_t ng_init(const char* resource_file, const char* title, ngine_t** core)
{
    status_t status = NG_OK;

    *core = (ngine_t*)calloc(1, sizeof(struct ngine));
    if (! *core)
    {
        // SDL_Log("%s: error allocating memory.", FUNCTION_NAME);
        return NG_ERROR;
    }

    SDL_SetMainReady();

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        // SDL_Log("Unable to initialise SDL: %s", SDL_GetError());
        return NG_ERROR;
    }

    (*core)->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        176, 208,
        SDL_WINDOW_FULLSCREEN);
    if (! (*core)->window)
    {
        // SDL_Log("Could not create window: %s", SDL_GetError());
        return NG_ERROR;
    }

    (*core)->renderer = SDL_CreateRenderer((*core)->window, 0, SDL_RENDERER_SOFTWARE);
    if (! (*core)->renderer)
    {
        // SDL_Log("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow((*core)->window);
        return NG_ERROR;
    }
    if (0 != SDL_RenderSetIntegerScale((*core)->renderer, SDL_TRUE))
    {
        // SDL_Log("Could not enable integer scale: %s", SDL_GetError());
        status = NG_WARNING;
    }

    init_file_reader(resource_file);

    if (NG_OK != load_font((*core)))
    {
        return NG_ERROR;
    }

    return status;
}

status_t ng_update(ngine_t* core)
{
    status_t     status     = NG_OK;
    Uint32       delta_time = 0;
    const Uint8* keystate   = SDL_GetKeyboardState(NULL);
    SDL_Event    event;
    SDL_Rect     dst;
    Sint32       player_index;

    core->time_b = core->time_a;
    core->time_a = SDL_GetTicks();

    // Calculate delta time.
    if (core->time_a > core->time_b)
    {
        delta_time = core->time_a - core->time_b;
    }
    else
    {
        delta_time = core->time_b - core->time_a;
    }
    core->time_since_last_frame = delta_time;

    // Set-up basic controls/events.
    if (is_map_loaded(core))
    {
        // TODO: This should not be hard-coded!
        // Add scripting language?
        player_index                                          = core->map->active_entity - 1;
        core->map->entity[player_index].show_animation        = SDL_FALSE;
        core->map->entity[player_index].animation.first_frame = 1;
        core->map->entity[player_index].animation.fps         = 5;
        core->map->entity[player_index].animation.length      = 3;

        if (keystate[SDL_SCANCODE_UP])
        {
            core->map->entity[player_index].show_animation      = SDL_TRUE;
            core->map->entity[player_index].animation.offset_y  = 3;

            move_entity(&core->map->entity[player_index], 0, -2, core);
        }
        if (keystate[SDL_SCANCODE_DOWN])
        {
            core->map->entity[player_index].show_animation      = SDL_TRUE;
            core->map->entity[player_index].animation.offset_y  = 0;

            move_entity(&core->map->entity[player_index], 0, 2, core);
        }
        if (keystate[SDL_SCANCODE_LEFT])
        {
            core->map->entity[player_index].show_animation      = SDL_TRUE;
            core->map->entity[player_index].animation.offset_y  = 1;

            move_entity(&core->map->entity[player_index], -2, 0, core);
        }
        if (keystate[SDL_SCANCODE_RIGHT])
        {
            core->map->entity[player_index].show_animation      = SDL_TRUE;
            core->map->entity[player_index].animation.offset_y  = 2;

            move_entity(&core->map->entity[player_index], 2, 0, core);
        }
        // TODO: See above.
    }

    if (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_BACKSPACE:
                        status = NG_EXIT;
                        goto exit;
                    case SDLK_5:
                        trigger_action(core);
                        break;
                    case SDLK_9:
                        core->debug_mode = !core->debug_mode;
                        break;
                    default:
                        clear_display_text(core);
                        break;
                }
            }
            case SDL_KEYUP:
            {
                switch (event.key.keysym.sym)
                {
                    default:
                        break;
                }
            }
        }
    }

    update_camera(core);
    status = render_scene(core);
    if (NG_OK != status)
    {
        goto exit;
    }
    status = draw_scene(core);

exit:
    return status;
}

void ng_free(ngine_t *core)
{
    if (core->display_text)
    {
        free(core->display_text);
        core->display_text = NULL;
    }

    if (core->font_texture)
    {
        SDL_DestroyTexture(core->font_texture);
        core->font_texture = NULL;
    }

    if (core->render_target)
    {
        SDL_DestroyTexture(core->render_target);
        core->render_target = NULL;
    }

    if (core->window)
    {
        SDL_DestroyWindow(core->window);
    }

    if (core->renderer)
    {
        SDL_DestroyRenderer(core->renderer);
    }

    if (core)
    {
        free(core);
        core = NULL;
    }

    SDL_Quit();
}

status_t ng_load_map(const char* map_name, ngine_t* core)
{
    status_t status = NG_OK;

    if (is_map_loaded(core))
    {
        // SDL_Log("A map has already been loaded: unload map first.");
        return NG_WARNING;
    }

    // Load map file and allocate required memory.

    // [1] Map.
    core->map = (map_t*)calloc(1, sizeof(struct map));
    if (! core->map)
    {
        // SDL_Log("%s: error allocating memory.", FUNCTION_NAME);
        return NG_WARNING;
    }

    // [2] Tiled map.
    status = load_tiled_map(map_name, core);
    if (NG_OK != status)
    {
        free(core->map);
        goto exit;
    }

    // [3] Tiles.
    status = load_tiles(core);
    if (NG_OK != status)
    {
        goto exit;
    }

    // [4] Entities.
    status = load_entities(core);
    if (NG_OK != status)
    {
        goto exit;
    }

    // [5] Tileset.
    status = load_tileset(core);
    if (NG_OK != status)
    {
        goto exit;
    }

    // [6] Sprites.
    status = load_sprites(core);
    if (NG_OK != status)
    {
        goto exit;
    }

    // [7] Animated tiles.
    status = load_animated_tiles(core);
    if (NG_OK != status)
    {
        goto exit;
    }

    core->map->height = (Sint32)((Sint32)core->map->handle->height * get_tile_height(core->map->handle));
    core->map->width  = (Sint32)((Sint32)core->map->handle->width  * get_tile_width(core->map->handle));

exit:
    if (NG_OK != status)
    {
        ng_unload_map(core);
    }

    clear_display_text(core);
    core->is_map_loaded = SDL_TRUE;
    return status;
}

void ng_unload_map(ngine_t* core)
{
    Sint32 index;

    if (! is_map_loaded(core))
    {
        // SDL_Log("No map has been loaded.");
        return;
    }
    core->is_map_loaded = SDL_FALSE;

    if (core->map->layer_texture)
    {
        SDL_DestroyTexture(core->map->layer_texture);
        core->map->layer_texture = NULL;
    }

    if (core->map->animated_tile_texture)
    {
        SDL_DestroyTexture(core->map->animated_tile_texture);
        core->map->animated_tile_texture = NULL;
    }

    // Free up allocated memory in reverse order.

    // [7] Animated tiles.
    free(core->map->animated_tile);

    // [6] Sprites.
    if (core->map->sprite_count > 0)
    {
        for (index = 0; index < core->map->sprite_count; index += 1)
        {
            core->map->sprite[index].id = 0;

            if (core->map->sprite[index].texture)
            {
                SDL_DestroyTexture(core->map->sprite[index].texture);
                core->map->sprite[index].texture = NULL;
            }
        }
    }

    free(core->map->sprite);
    core->map->sprite = NULL;

    // [5] Tileset.
    if (core->map->tileset_texture)
    {
        SDL_DestroyTexture(core->map->tileset_texture);
        core->map->tileset_texture = NULL;
    }

    // [4] Entities.
    free(core->map->entity);

    // [3] Tiles.
    free(core->map->tile_desc);
    core->map->tile_desc = NULL;

    // [2] Tiled map.
    unload_tiled_map(core);

    // [1] Map.
    free(core->map);
    core->map = NULL;
}
