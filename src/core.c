// Spdx-License-Identifier: MIT

#include <SDL.h>
#include "core.h"

static status_t update_scene(core_t* core)
{
    status_t status = CORE_OK;
    Sint32   index;

    status = update_map(core);
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
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
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
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    dst.x = 0;
    dst.y = 0;
    dst.w = 176;
    dst.h = 208;

    if (0 > SDL_RenderCopy(core->renderer, core->render_target, NULL, &dst))
    {
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    SDL_SetRenderDrawColor(core->renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderPresent(core->renderer);
    SDL_RenderClear(core->renderer);

    return CORE_OK;
}

static void restrict_camera(core_t* core)
{
    if (! is_map_loaded(core))
    {
        return;
    }

    core->camera.pos_x = SDL_clamp(core->camera.pos_x, 0, core->map->width  - 176);
    core->camera.pos_y = SDL_clamp(core->camera.pos_y, 0, core->map->height - 208);

    if (core->camera.target_actor_id)
    {
        if (core->camera.target_actor_id < 1)
        {
            core->camera.target_actor_id = core->map->actor_count;
        }
        else if (core->camera.target_actor_id > core->map->actor_count)
        {
            core->camera.target_actor_id = 1;
        }
    }
}

static void update_camera(core_t* core)
{
    if (! is_map_loaded(core))
    {
        return;
    }

    if (core->camera.is_locked)
    {
        if (core->camera.target_actor_id)
        {
            actor_t* target = &core->map->actor[core->camera.target_actor_id - 1];

            core->camera.pos_x  = target->pos_x;
            core->camera.pos_x -= 88;  // 176 / 2
            core->camera.pos_y  = target->pos_y;
            core->camera.pos_y -= 104; // 208 / 2
        }

        if (core->camera.pos_x < 0)
        {
            core->camera.pos_x = 0;
        }

        restrict_camera(core);
    }
}

static void move_actor(actor_t* actor, Sint32 offset_x, Sint32 offset_y, core_t* core)
{
    Sint32 tile_index;
    Sint32 adjacent_tile;

    if (! is_map_loaded(core))
    {
        return;
    }

    tile_index = get_tile_index(actor->pos_x, actor->pos_y, core);

    // Moves right.
    if (offset_x > 0)
    {
        adjacent_tile = tile_index + 1;
        if (! core->map->tile_desc[adjacent_tile].is_solid)
        {
            actor->pos_x += offset_x;
        }
    }
    // Moves left.
    else if (offset_x < 0)
    {
        adjacent_tile = tile_index - 1;
        if (! core->map->tile_desc[adjacent_tile].is_solid)
        {
            actor->pos_x += offset_x;
        }
    }

    // Moves down.
    if (offset_y > 0)
    {
        adjacent_tile = tile_index + core->map->handle->width;
        if (adjacent_tile >= core->map->tile_desc_count)
        {
            adjacent_tile = core->map->tile_desc_count - 1;
        }

        if (! core->map->tile_desc[adjacent_tile].is_solid)
        {
            actor->pos_y += offset_y;
        }
    }
    // Moves up.
    else if (offset_y < 0)
    {
        adjacent_tile = tile_index - core->map->handle->width;
        if (adjacent_tile >= 0)
        {
            if (! core->map->tile_desc[adjacent_tile].is_solid)
            {
                actor->pos_y += offset_y;
            }
        }
        else
        {
            actor->pos_y += offset_y;
        }
    }
}

status_t init_core(const char* resource_file, const char* title, core_t** core)
{
    status_t status = CORE_OK;

    *core = (core_t*)calloc(1, sizeof(struct core));
    if (! *core)
    {
        dbgprint("%s: error allocating memory.", FUNCTION_NAME);
        return CORE_ERROR;
    }

    SDL_SetMainReady();

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        dbgprint("Unable to initialise SDL: %s", SDL_GetError());
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
        dbgprint("Could not create window: %s", SDL_GetError());
        return CORE_ERROR;
    }

    (*core)->renderer = SDL_CreateRenderer((*core)->window, 0, SDL_RENDERER_SOFTWARE);
    if (! (*core)->renderer)
    {
        dbgprint("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow((*core)->window);
        return CORE_ERROR;
    }
    if (0 != SDL_RenderSetIntegerScale((*core)->renderer, SDL_TRUE))
    {
        dbgprint("Could not enable integer scale: %s", SDL_GetError());
        status = CORE_WARNING;
    }

    init_file_reader(resource_file);
    (*core)->is_active = SDL_TRUE;

    return status;
}

status_t update_core(core_t* core)
{
    status_t     status     = CORE_OK;
    Uint32       delta_time = 0;
    const Uint8* keystate   = SDL_GetKeyboardState(NULL);
    SDL_Event    event;
    SDL_Rect     dst;
    Sint32       target_index;

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
        target_index                                         = core->camera.target_actor_id - 1;
        core->map->actor[target_index].show_animation        = SDL_FALSE;
        core->map->actor[target_index].animation.first_frame = 1;
        core->map->actor[target_index].animation.fps         = 5;
        core->map->actor[target_index].animation.length      = 3;

        if (keystate[SDL_SCANCODE_UP])
        {
            core->map->actor[target_index].show_animation      = SDL_TRUE;
            core->map->actor[target_index].animation.offset_y  = 3;

            move_actor(&core->map->actor[target_index], 0, -2, core);
        }
        if (keystate[SDL_SCANCODE_DOWN])
        {
            core->map->actor[target_index].show_animation      = SDL_TRUE;
            core->map->actor[target_index].animation.offset_y  = 0;

            move_actor(&core->map->actor[target_index], 0, 2, core);
        }
        if (keystate[SDL_SCANCODE_LEFT])
        {
            core->map->actor[target_index].show_animation      = SDL_TRUE;
            core->map->actor[target_index].animation.offset_y  = 1;

            move_actor(&core->map->actor[target_index], -2, 0, core);
        }
        if (keystate[SDL_SCANCODE_RIGHT])
        {
            core->map->actor[target_index].show_animation      = SDL_TRUE;
            core->map->actor[target_index].animation.offset_y  = 2;

            move_actor(&core->map->actor[target_index], 2, 0, core);
        }
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
                    case SDLK_1:
                        unload_map(core);
                        goto exit;
                    case SDLK_2:
                        if (CORE_ERROR == load_map("entry.tmj", core))
                        {
                            goto exit;
                        }
                        break;
                    case SDLK_9:
                        core->debug_mode = !core->debug_mode;
                        break;
                    case SDLK_3:
                        core->camera.target_actor_id += 1;
                        break;
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

    update_camera(core);
    status = update_scene(core);
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

status_t load_map(const char* map_name, core_t* core)
{
    status_t status = CORE_OK;

    if (is_map_loaded(core))
    {
        dbgprint("A map has already been loaded: unload map first.");
        return CORE_WARNING;
    }

    // Load map file and allocate required memory.

    // [1] Map.
    core->map = (map_t*)calloc(1, sizeof(struct map));
    if (! core->map)
    {
        dbgprint("%s: error allocating memory.", FUNCTION_NAME);
        return CORE_WARNING;
    }

    // [2] Tiled map.
    status = load_tiled_map(map_name, core);
    if (CORE_OK != status)
    {
        free(core->map);
        goto exit;
    }
    core->is_map_loaded = SDL_TRUE;

    // [3] Tiles.
    status = load_tiles(core);
    if (CORE_OK != status)
    {
        goto exit;
    }

    // [4] Actors.
    status = load_actors(core);
    if (CORE_OK != status)
    {
        goto exit;
    }

    // [5] Tileset.
    status = load_tileset(core);
    if (CORE_OK != status)
    {
        goto exit;
    }

    // [6] Sprites.
    status = load_sprites(core);
    if (CORE_OK != status)
    {
        goto exit;
    }

    // [7] Animated tiles.
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
    Sint32 index;

    if (! is_map_loaded(core))
    {
        dbgprint("No map has been loaded.");
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
    if (0 < core->map->sprite_count)
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

    // [4] Actors.
    free(core->map->actor);

    // [3] Tiles.
    free(core->map->tile_desc);
    core->map->tile_desc = NULL;

    // [2] Tiled map.
    unload_tiled_map(core);

    // [1] Map.
    free(core->map);
    core->map = NULL;
}

status_t load_map_ex(const char* map_name, Sint32 actor_id, Sint32 pos_x, Sint32 pos_y, core_t* core)
{
    status_t status = CORE_OK;
    actor_t* target_actor;

    status = load_map(map_name, core);
    if (CORE_OK != status)
    {
        return status;
    }

    if (actor_id > core->map->actor_count)
    {
        status = CORE_ERROR;
        goto exit;
    }

    core->camera.target_actor_id = actor_id;
    target_actor                 = &core->map->actor[actor_id - 1];
    target_actor->pos_x          = pos_x;
    target_actor->pos_y          = pos_y;

exit:
    return status;
}
