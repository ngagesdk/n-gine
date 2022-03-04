// SPDX-License-Identifier: MIT

#include <SDL.h>
#include <cwalk.h>
#include "core.h"

#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>

#if defined __SYMBIAN32__
#define CUTE_TILED_SNPRINTF(ARGS...) (void)(ARGS)
#define CUTE_TILED_STRTOLL        strtol
#define CUTE_TILED_STRTOULL       strtoul
#define STRPOOL_EMBEDDED_STRNICMP strncasecmp
#endif

#define CUTE_TILED_IMPLEMENTATION
#include <cute_tiled.h>

#define H_objectgroup 0xc0b9d518970be349
#define H_tilelayer   0x0377d9f70e844fb0

#define ANIM_TILE_FPS 15

Sint32 get_first_gid(cute_tiled_map_t* tiled_map)
{
    return tiled_map->tilesets->firstgid;
}

cute_tiled_layer_t* get_head_layer(cute_tiled_map_t* tiled_map)
{
    return tiled_map->layers;
}

SDL_bool is_tiled_layer_of_type(const tiled_layer_type tiled_type, cute_tiled_layer_t* tiled_layer, core_t* core)
{
    switch (tiled_type)
    {
        case TILE_LAYER:
            if (core->map->hash_id_tilelayer == tiled_layer->type.hash_id)
            {
                return SDL_TRUE;
            }
            break;
        case OBJECT_GROUP:
            if (core->map->hash_id_objectgroup == tiled_layer->type.hash_id)
            {
                return SDL_TRUE;
            }
            break;
    }

    return SDL_FALSE;
}

cute_tiled_object_t* get_head_object(cute_tiled_layer_t* tiled_layer, core_t* core)
{
    if (is_tiled_layer_of_type(OBJECT_GROUP, tiled_layer, core))
    {
        return tiled_layer->objects;
    }

    return NULL;
}

cute_tiled_tileset_t* get_head_tileset(cute_tiled_map_t* tiled_map)
{
    return tiled_map->tilesets;
}

Sint32* get_layer_content(cute_tiled_layer_t* tiled_layer)
{
    return (Sint32*)tiled_layer->data;
}

const char* get_layer_name(cute_tiled_layer_t* tiled_layer)
{
    return tiled_layer->name.ptr;
}

Sint32 get_layer_property_count(cute_tiled_layer_t* tiled_layer)
{
    return tiled_layer->property_count;
}

Sint32 get_local_id(Sint32 gid, cute_tiled_map_t* tiled_map)
{
    Sint32 local_id = gid - get_first_gid(tiled_map);
    return local_id >= 0 ? local_id : 0;
}

Sint32 get_map_property_count(cute_tiled_map_t* tiled_map)
{
    return tiled_map->property_count;
}

Sint32 get_next_animated_tile_id(Sint32 gid, Sint32 current_frame, cute_tiled_map_t* tiled_map)
{
    cute_tiled_tileset_t*         tileset = get_head_tileset(tiled_map);
    cute_tiled_tile_descriptor_t* tile    = tileset->tiles;

    while (tile)
    {
        if (tile->tile_index == gid)
        {
            return tile->animation[current_frame].tileid;
        }
        tile = tile->next;
    }

    return 0;
}

const char* get_object_name(cute_tiled_object_t* tiled_object)
{
    return tiled_object->name.ptr;
}

Sint32 get_object_property_count(cute_tiled_object_t* tiled_object)
{
    return tiled_object->property_count;
}

const char* get_object_type_name(cute_tiled_object_t* tiled_object)
{
    return tiled_object->type.ptr;
}

int get_tile_height(cute_tiled_map_t* tiled_map)
{
    return tiled_map->tilesets->tileheight;
}

void get_tile_position(Sint32 gid, Sint32* pos_x, Sint32* pos_y, cute_tiled_map_t* tiled_map)
{
    cute_tiled_tileset_t* tileset  = tiled_map->tilesets;
    Sint32                local_id = get_local_id(gid, tiled_map);

    *pos_x = (local_id % tileset->columns) * get_tile_width(tiled_map);
    *pos_y = (local_id / tileset->columns) * get_tile_height(tiled_map);
}

Sint32 get_tile_property_count(cute_tiled_tile_descriptor_t* tiled_tile)
{
    return tiled_tile->property_count;
}

int get_tile_width(cute_tiled_map_t* tiled_map)
{
    return tiled_map->tilesets->tilewidth;
}

SDL_bool is_gid_valid(Sint32 gid, cute_tiled_map_t* tiled_map)
{
    if (gid)
    {
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

SDL_bool is_tile_animated(Sint32 gid, Sint32* animation_length, Sint32* id, cute_tiled_map_t* tiled_map)
{
    Sint32                        local_id = get_local_id(gid, tiled_map);
    cute_tiled_tileset_t*         tileset  = tiled_map->tilesets;
    cute_tiled_tile_descriptor_t* tile     = tileset->tiles;

    while (tile)
    {
        if (tile->tile_index == local_id)
        {
            if (tile->animation)
            {
                if (animation_length)
                {
                    *animation_length = tile->frame_count;
                }
                if (id)
                {
                    *id = tile->animation->tileid;
                }
                return SDL_TRUE;
            }
        }
        tile = tile->next;
    }

    return SDL_FALSE;
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

void load_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, core_t* core)
{
    int index = 0;

    for (index = 0; index < property_count; index += 1)
    {
        if (name_hash == generate_hash((const unsigned char*)properties[index].name.ptr))
        {
            break;
        }
    }

    if (properties[index].name.ptr)
    {
        switch (properties[index].type)
        {
            case CUTE_TILED_PROPERTY_COLOR:
            case CUTE_TILED_PROPERTY_FILE:
            case CUTE_TILED_PROPERTY_NONE:
                break;
            case CUTE_TILED_PROPERTY_INT:
                SDL_Log("Loading integer property '%s': %d", properties[index].name.ptr, properties[index].data.integer);

                core->map->integer_property = properties[index].data.integer;
                break;
            case CUTE_TILED_PROPERTY_BOOL:
                SDL_Log("Loading boolean property '%s': %u", properties[index].name.ptr, properties[index].data.boolean);

                core->map->boolean_property = (SDL_bool)properties[index].data.boolean;
                break;
            case CUTE_TILED_PROPERTY_FLOAT:
                SDL_Log("Loading decimal property '%s': %f", properties[index].name.ptr, (float)properties[index].data.floating);

                core->map->decimal_property = (float)properties[index].data.floating;
                break;
            case CUTE_TILED_PROPERTY_STRING:
                SDL_Log("Loading string property '%s': %s", properties[index].name.ptr, properties[index].data.string.ptr);

                core->map->string_property  = properties[index].data.string.ptr;
                break;
        }
    }
}

status_t load_tiled_map(const char* map_file_name, core_t* core)
{
    FILE*               fp = fopen(map_file_name, "r");
    cute_tiled_layer_t* layer;

    if (fp)
    {
        fclose(fp);
    }
    else
    {
        SDL_Log("%s: %s not found.", FUNCTION_NAME, map_file_name);
        return CORE_WARNING;
    }

    core->map->handle = (cute_tiled_map_t*)cute_tiled_load_map_from_file(map_file_name, NULL);
    if (! core->map->handle)
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, cute_tiled_error_reason);
        return CORE_WARNING;
    }

    layer = get_head_layer(core->map->handle);
    while (layer)
    {
        if (H_tilelayer == generate_hash((const unsigned char*)layer->type.ptr) && !core->map->hash_id_tilelayer)
        {
            core->map->hash_id_tilelayer = layer->type.hash_id;
            SDL_Log("Set hash ID for tile layer: %llu", core->map->hash_id_tilelayer);
        }
        else if (H_objectgroup == generate_hash((const unsigned char*)layer->type.ptr) && !core->map->hash_id_objectgroup)
        {
            core->map->hash_id_objectgroup = layer->type.hash_id;
            SDL_Log("Set hash ID for object group: %llu", core->map->hash_id_objectgroup);
        }
        layer = layer->next;
    }

    return CORE_OK;
}

Sint32 remove_gid_flip_bits(Sint32 gid)
{
    return cute_tiled_unset_flags(gid);
}

SDL_bool tile_has_properties(Sint32 gid, cute_tiled_tile_descriptor_t** tile, cute_tiled_map_t* tiled_map)
{
    Sint32 local_id;

    local_id = gid - get_first_gid(tiled_map);

    while ((*tile))
    {
        if ((*tile)->tile_index == local_id)
        {
            if (0 < (*tile)->property_count)
            {
                return SDL_TRUE;
            }
        }
        (*tile) = (*tile)->next;
    }

    return SDL_FALSE;
}

void unload_tiled_map(core_t* core)
{
    core->map->hash_id_objectgroup = 0;
    core->map->hash_id_tilelayer   = 0;

    if (core->map->handle)
    {
        cute_tiled_free_map(core->map->handle);
    }
}

SDL_bool is_map_loaded(core_t* core)
{
    if (core->is_map_loaded)
    {
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

SDL_bool get_boolean_map_property(const Uint64 name_hash, core_t* core)
{
    Sint32 prop_cnt;

    if (! is_map_loaded(core))
    {
        return SDL_FALSE;
    }

    prop_cnt                    = get_map_property_count(core->map->handle);
    core->map->boolean_property = SDL_FALSE;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->boolean_property;
}

float get_decimal_map_property(const Uint64 name_hash, core_t* core)
{
    Sint32 prop_cnt;

    if (! is_map_loaded(core))
    {
        return 0.0;
    }

    prop_cnt                    = get_map_property_count(core->map->handle);
    core->map->decimal_property = 0.0;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->decimal_property;
}

Sint32 get_integer_map_property(const Uint64 name_hash, core_t* core)
{
    Sint32 prop_cnt;

    if (! is_map_loaded(core))
    {
        return 0;
    }

    prop_cnt                    = get_map_property_count(core->map->handle);
    core->map->integer_property = 0;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->integer_property;
}

const char* get_string_map_property(const Uint64 name_hash, core_t* core)
{
    Sint32 prop_cnt;

    if (! is_map_loaded(core))
    {
        return NULL;
    }

    prop_cnt                   = get_map_property_count(core->map->handle);
    core->map->string_property = NULL;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->string_property;
}

status_t load_texture_from_file(const char* file_name, SDL_Texture** texture, core_t* core)
{
    SDL_Surface* surface;

    if (! file_name)
    {
        return CORE_WARNING;
    }

    surface = SDL_LoadBMP(file_name);
    if (! surface)
    {
        SDL_Log("Failed to load image: %s", SDL_GetError());
        return CORE_ERROR;
    }
    if (0 != SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0xff, 0x00, 0xff)))
    {
        SDL_Log("Failed to set color key for %s: %s", file_name, SDL_GetError());
    }
    if (0 != SDL_SetSurfaceRLE(surface, 1))
    {
        SDL_Log("Could not enable RLE for surface %s: %s", file_name, SDL_GetError());
    }

    *texture = SDL_CreateTextureFromSurface(core->renderer, surface);
    if (! *texture)
    {
        SDL_Log("Could not create texture from surface: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return CORE_ERROR;
    }
    SDL_FreeSurface(surface);

    SDL_Log("Loading image from file: %s.", file_name);

    return CORE_OK;
}

status_t load_tileset(const char* tileset_file_name, core_t* core)
{
    status_t status = CORE_OK;

    if (CORE_OK != load_texture_from_file(tileset_file_name, &core->map->tileset_texture, core))
    {
        SDL_Log("%s: Error loading image '%s'.", FUNCTION_NAME, tileset_file_name);
        status = CORE_ERROR;
    }

warning:
    return status;
}

status_t load_animated_tiles(core_t* core)
{
    cute_tiled_layer_t* layer               = get_head_layer(core->map->handle);
    Sint32              animated_tile_count = 0;
    Sint32              index_height        = 0;
    Sint32              index_width         = 0;

    while (layer)
    {
        if (is_tiled_layer_of_type(TILE_LAYER, layer, core) && layer->visible)
        {
            for (index_height = 0; index_height < (Sint32)core->map->handle->height; index_height += 1)
            {
                for (index_width = 0; index_width < (Sint32)core->map->handle->width; index_width += 1)
                {
                    Sint32* layer_content = get_layer_content(layer);
                    Sint32  gid           = remove_gid_flip_bits((Sint32)layer_content[(index_height * (Sint32)core->map->handle->width) + index_width]);

                    if (is_tile_animated(gid, NULL, NULL, core->map->handle))
                    {
                        animated_tile_count += 1;
                    }
                }
            }
        }
        layer = layer->next;
    }

    if (0 >= animated_tile_count)
    {
        return CORE_OK;
    }
    else
    {
        core->map->animated_tile = (animated_tile_t*)calloc((size_t)animated_tile_count, sizeof(struct animated_tile));
        if (! core->map->animated_tile)
        {
            SDL_Log("%s: error allocating memory.", FUNCTION_NAME);
            return CORE_ERROR;
        }
    }

    SDL_Log("Load %u animated tile(s).", animated_tile_count);

    return CORE_OK;
}

status_t create_and_set_render_target(SDL_Texture** target, core_t* core)
{
    if (! (*target))
    {
        (*target) = SDL_CreateTexture(
            core->renderer,
            SDL_PIXELFORMAT_RGB444,
            SDL_TEXTUREACCESS_TARGET,
            176,
            208);
    }

    if (! (*target))
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }
    else
    {
        if (0 > SDL_SetTextureBlendMode((*target), SDL_BLENDMODE_NONE))
        {
            SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
            SDL_DestroyTexture((*target));
            return CORE_ERROR;
        }
    }

    if (0 > SDL_SetRenderTarget(core->renderer, (*target)))
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        SDL_DestroyTexture((*target));
        return CORE_ERROR;
    }

    SDL_RenderClear(core->renderer);

    return CORE_OK;
}

SDL_bool get_boolean_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, core_t* core)
{
    core->map->boolean_property = SDL_FALSE;
    load_property(name_hash, properties, property_count, core);
    return core->map->boolean_property;
}

float get_decimal_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, core_t* core)
{
    core->map->decimal_property = 0.0;
    load_property(name_hash, properties, property_count, core);
    return core->map->decimal_property;
}

int32_t get_integer_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, core_t* core)
{
    core->map->integer_property = 0;
    load_property(name_hash, properties, property_count, core);
    return core->map->integer_property;
}

const char* get_string_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, core_t* core)
{
    core->map->string_property = NULL;
    load_property(name_hash, properties, property_count, core);
    return core->map->string_property;
}

status_t render_map(core_t* core)
{
    cute_tiled_layer_t* layer;
    Sint32              index;

    if (! core->is_map_loaded)
    {
        return CORE_OK;
    }

    // Update and render animated tiles.
    core->map->time_since_last_anim_frame += core->time_since_last_frame;

    if (0 < core->map->animated_tile_index && core->map->time_since_last_anim_frame >= (Uint32)(1000 / ANIM_TILE_FPS))
    {
        core->map->time_since_last_anim_frame = 0;

        if (0 > SDL_SetRenderTarget(core->renderer, core->map->layer_texture))
        {
            SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return CORE_ERROR;
        }

        for (index = 0; core->map->animated_tile_index > index; index += 1)
        {
            cute_tiled_tileset_t* tileset;
            Sint32                gid          = core->map->animated_tile[index].gid;
            Sint32                next_tile_id = 0;
            Sint32                local_id;
            SDL_Rect              dst;
            SDL_Rect              src;

            local_id = core->map->animated_tile[index].id + 1;
            tileset  = get_head_tileset(core->map->handle);
            src.w    = dst.w = get_tile_width(core->map->handle);
            src.h    = dst.h = get_tile_height(core->map->handle);
            dst.x    = core->map->animated_tile[index].dst_x;
            dst.y    = core->map->animated_tile[index].dst_y;

            get_tile_position(local_id, (Uint32*)&src.x, (Uint32*)&src.y, core->map->handle);

            if (0 > SDL_RenderCopy(core->renderer, core->map->tileset_texture, &src, &dst))
            {
                SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
                return CORE_ERROR;
            }

            core->map->animated_tile[index].current_frame += 1;

            if (core->map->animated_tile[index].current_frame >= core->map->animated_tile[index].animation_length)
            {
                core->map->animated_tile[index].current_frame = 0;
            }

            next_tile_id = get_next_animated_tile_id(gid, core->map->animated_tile[index].current_frame, core->map->handle);

            core->map->animated_tile[index].id = next_tile_id;
        }
    }

    layer = get_head_layer(core->map->handle);

    if (CORE_OK != create_and_set_render_target(&core->render_target, core))
    {
        return CORE_ERROR;
    }

    // Texture has already been rendered.
    if (core->map->layer_texture)
    {
        Sint32   render_pos_x = core->map->pos_x - core->camera.pos_x;
        Sint32   render_pos_y = core->map->pos_y - core->camera.pos_y;
        SDL_Rect dst          = {
            (Sint32)render_pos_x,
            (Sint32)render_pos_y,
            (Sint32)core->map->width,
            (Sint32)core->map->height
        };

        if (0 > SDL_RenderCopyEx(core->renderer, core->map->layer_texture, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
        {
            SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return CORE_ERROR;
        }

        return CORE_OK;
    }

    // Texture does not yet exist. Render it!
    core->map->layer_texture = SDL_CreateTexture(
        core->renderer,
        SDL_PIXELFORMAT_RGB444,
        SDL_TEXTUREACCESS_TARGET,
        (Sint32)core->map->width,
        (Sint32)core->map->height);

    if (! core->map->layer_texture)
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    if (0 > SDL_SetRenderTarget(core->renderer, core->map->layer_texture))
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }
    SDL_RenderClear(core->renderer);

    while (layer)
    {
        SDL_Rect dst;
        SDL_Rect src;

        if (is_tiled_layer_of_type(TILE_LAYER, layer, core))
        {
            Sint32 prop_cnt = get_layer_property_count(layer);

            if (layer->visible)
            {
                Sint32 index_height;
                Sint32 index_width;

                for (index_height = 0; index_height < (Sint32)core->map->handle->height; index_height += 1)
                {
                    for (index_width = 0; index_width < (Sint32)core->map->handle->width; index_width += 1)
                    {
                        Sint32* layer_content = get_layer_content(layer);
                        Sint32  gid           = remove_gid_flip_bits((Sint32)layer_content[(index_height * (Sint32)core->map->handle->width) + index_width]);
                        Sint32  local_id      = gid - get_first_gid(core->map->handle);

                        if (is_gid_valid(gid, core->map->handle))
                        {
                            cute_tiled_tileset_t* tileset          = get_head_tileset(core->map->handle);
                            Sint32                animation_length = 0;
                            Sint32                id               = 0;

                            src.w = dst.w = get_tile_width(core->map->handle);
                            src.h = dst.h = get_tile_height(core->map->handle);
                            dst.x = (Sint32)(index_width  * get_tile_width(core->map->handle));
                            dst.y = (Sint32)(index_height * get_tile_height(core->map->handle));

                            get_tile_position(gid, (Uint32*)&src.x, (Uint32*)&src.y, core->map->handle);
                            SDL_RenderCopy(core->renderer, core->map->tileset_texture, &src, &dst);

                            if (is_tile_animated(gid, &animation_length, &id, core->map->handle))
                            {
                                core->map->animated_tile[core->map->animated_tile_index].gid              = get_local_id(gid, core->map->handle);
                                core->map->animated_tile[core->map->animated_tile_index].id               = id;
                                core->map->animated_tile[core->map->animated_tile_index].dst_x            = dst.x;
                                core->map->animated_tile[core->map->animated_tile_index].dst_y            = dst.y;
                                core->map->animated_tile[core->map->animated_tile_index].current_frame    = 0;
                                core->map->animated_tile[core->map->animated_tile_index].animation_length = animation_length;

                                core->map->animated_tile_index += 1;
                            }
                        }
                    }
                }

                {
                    const char* layer_name = get_layer_name(layer);
                    SDL_Log("Render map layer: %s", layer_name);
                }
            }
        }
        layer = layer->next;
    }

    if (0 > SDL_SetRenderTarget(core->renderer, core->render_target))
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    if (0 > SDL_SetTextureBlendMode(core->map->layer_texture, SDL_BLENDMODE_NONE))
    {
        SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    return CORE_OK;
}
