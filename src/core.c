// Spdx-License-Identifier: MIT

#include <SDL.h>
#include "core.h"

#define SPRITE_COUNT 1

#if defined __SYMBIAN32__
#  define MAP_FILE_NAME     "E:\\entry.tmj"
#  define TILESET_FILE_NAME "E:\\lowtown.bmp"
#  define SPRITE_1          "E:\\morgan.bmp"
#else
#  define MAP_FILE_NAME     "entry.tmj"
#  define TILESET_FILE_NAME "lowtown.bmp"
#  define SPRITE_1          "morgan.bmp"
#endif

static status_t render_scene(core_t* core)
{
    status_t status = CORE_OK;
    Sint32   index;

    status = render_map(core);
    if (CORE_OK != status)
    {
        return status;
    }

    return status;
}

static status_t draw_scene(core_t* core)
{
    SDL_Rect dst;
    Sint32   index;

    if (0 > SDL_SetRenderTarget(core->renderer, NULL))
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
    }

    if (! core->is_map_loaded)
    {
        SDL_SetRenderDrawColor(core->renderer, 0x00, 0x00, 0xaa, 0x00);
        SDL_RenderPresent(core->renderer);
        SDL_RenderClear(core->renderer);

        return CORE_OK;
    }

    if (0 > SDL_RenderCopy(core->renderer, core->render_target, NULL, &dst))
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    dst.x = 0;
    dst.y = 0;
    dst.w = 176;
    dst.h = 208;

    if (0 > SDL_RenderCopy(core->renderer, core->render_target, NULL, &dst))
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    SDL_SetRenderDrawColor(core->renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderPresent(core->renderer);
    SDL_RenderClear(core->renderer);

    return CORE_OK;
}

status_t init_core(const char* title, core_t** core)
{
    status_t status = CORE_OK;

    *core = (core_t*)calloc(1, sizeof(struct core));
    if (! *core)
    {
        SDL_Log("%s: error allocating memory.", __FUNCTION__);
        return CORE_ERROR;
    }

    SDL_SetMainReady();

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Unable to initialise SDL: %s", SDL_GetError());
        return CORE_ERROR;
    }

    (*core)->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        176, 208,
        SDL_WINDOW_FULLSCREEN);
    if (! (*core)->window)
    {
        SDL_Log("Could not create window: %s", SDL_GetError());
        return CORE_ERROR;
    }

    (*core)->renderer = SDL_CreateRenderer((*core)->window, 0, SDL_RENDERER_SOFTWARE);
    if (! (*core)->renderer)
    {
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow((*core)->window);
        return CORE_ERROR;
    }
    if (0 != SDL_RenderSetIntegerScale((*core)->renderer, SDL_TRUE))
    {
        SDL_Log("Could not enable integer scale: %s", SDL_GetError());
        status = CORE_WARNING;
    }

    (*core)->is_active = SDL_TRUE;

    return status;
}

status_t update_core(core_t* core)
{
    status_t     status     = CORE_OK;
    Uint32       delta_time = 0;
    Uint8        offset     = 7;
    const Uint8* keystate   = SDL_GetKeyboardState(NULL);
    SDL_Event    event;
    SDL_Rect     dst;

    if (keystate[SDL_SCANCODE_5])
    {
        offset = 0;
    }

    if (keystate[SDL_SCANCODE_UP])
    {
        core->camera.pos_y -= 1 + offset;
    }
    if (keystate[SDL_SCANCODE_DOWN])
    {
        core->camera.pos_y += 1 + offset;
    }
    if (keystate[SDL_SCANCODE_LEFT])
    {
        core->camera.pos_x -= 1 + offset;
    }
    if (keystate[SDL_SCANCODE_RIGHT])
    {
        core->camera.pos_x += 1 + offset;
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
                        status = CORE_EXIT;
                        goto exit;
                    default:
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

    if (! is_map_loaded(core))
    {
        return;
    }

    core->time_b = core->time_a;
    core->time_a = SDL_GetTicks();

    if (core->time_a > core->time_b)
    {
        delta_time = core->time_a - core->time_b;
    }
    else
    {
        delta_time = core->time_b - core->time_a;
    }
    core->time_since_last_frame = delta_time;

    core->camera.pos_x = SDL_clamp(core->camera.pos_x, 0, core->map->width  - 176);
    core->camera.pos_y = SDL_clamp(core->camera.pos_y, 0, core->map->height - 208);

    status = render_scene(core);
    if (CORE_OK != status)
    {
        goto exit;
    }
    status = draw_scene(core);

exit:
    return status;
}

void free_core(core_t *core)
{
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

status_t load_map(core_t* core)
{
    status_t status = CORE_OK;

    if (is_map_loaded(core))
    {
        SDL_Log("A map has already been loaded: unload map first.");
        return CORE_WARNING;
    }

    // Load map file and allocate required memory.

    // [1] Map.
    core->map = (map_t*)calloc(1, sizeof(struct map));
    if (! core->map)
    {
        SDL_Log("%s: error allocating memory.", __FUNCTION__);
        return CORE_WARNING;
    }

    // [2] Tiled map.
    status = load_tiled_map(MAP_FILE_NAME, core);
    if (CORE_OK != status)
    {
        free(core->map);
        goto exit;
    }
    core->is_map_loaded = SDL_TRUE;

    // [3] Actors.
    status = load_actors(core);
    if (CORE_OK != status)
    {
        goto exit;
    }

    /* DEBUG */
    core->map->actor[0].show_animation          = SDL_TRUE;
    core->map->actor[0].animation.first_frame   = 1;
    core->map->actor[0].animation.fps           = 8;
    core->map->actor[0].animation.length        = 3;
    core->map->actor[0].animation.offset_y      = 0;
    /* DEBUG */

    // [4] Tileset.
    status = load_tileset(TILESET_FILE_NAME, core);
    if (CORE_OK != status)
    {
        goto exit;
    }

    // [5] Sprites.
    status = alloc_sprites(SPRITE_COUNT, core);
    if (CORE_OK != status)
    {
        goto exit;
    }

    status = load_sprite(SPRITE_1, 1, core);
    if (CORE_OK != status)
    {
        goto exit;
    }

    // [6] Animated tiles.
    status = load_animated_tiles(core);
    if (CORE_OK != status)
    {
        goto exit;
    }

    core->map->height = (Sint32)((Sint32)core->map->handle->height * get_tile_height(core->map->handle));
    core->map->width  = (Sint32)((Sint32)core->map->handle->width  * get_tile_width(core->map->handle));

exit:
    if (CORE_OK != status)
    {
        unload_map(core);
    }
    return status;
}

void unload_map(core_t* core)
{
    if (! is_map_loaded(core))
    {
        SDL_Log("No map has been loaded.");
        return;
    }
    core->is_map_loaded = SDL_FALSE;

    if (core->map->layer_texture)
    {
        SDL_DestroyTexture(core->map->layer_texture);
        core->map->layer_texture = NULL;
    }

    if (core->render_target)
    {
        SDL_DestroyTexture(core->render_target);
        core->render_target = NULL;
    }

    if (core->map->animated_tile_texture)
    {
        SDL_DestroyTexture(core->map->animated_tile_texture);
        core->map->animated_tile_texture = NULL;
    }

    // Free up allocated memory in reverse order.

    // [6] Animated tiles.
    free(core->map->animated_tile);

    // [5] Sprites.
    dealloc_sprites(core);

    // [4] Tileset.
    if (core->map->tileset_texture)
    {
        SDL_DestroyTexture(core->map->tileset_texture);
        core->map->tileset_texture = NULL;
    }

    // [3] Actors.
    free(core->map->actor);

    // [2] Tiled map.
    unload_tiled_map(core);

    // [1] Map.
    free(core->map);
}
