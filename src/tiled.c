// SPDX-License-Identifier: MIT

#include <SDL.h>
#include <cwalk.h>
#include "ngine.h"
#include "utils.h"

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

#define ANIM_TILE_FPS 15

/* PRIVATE FUNCTIONS */

static Sint32 get_first_gid(cute_tiled_map_t* tiled_map)
{
    return tiled_map->tilesets->firstgid;
}

static cute_tiled_layer_t* get_head_layer(cute_tiled_map_t* tiled_map)
{
    return tiled_map->layers;
}

static SDL_bool is_tiled_layer_of_type(const tiled_layer_type tiled_type, cute_tiled_layer_t* tiled_layer, ngine_t* core)
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

static cute_tiled_object_t* get_head_object(cute_tiled_layer_t* tiled_layer, ngine_t* core)
{
    if (is_tiled_layer_of_type(OBJECT_GROUP, tiled_layer, core))
    {
        return tiled_layer->objects;
    }

    return NULL;
}

static cute_tiled_tileset_t* get_head_tileset(cute_tiled_map_t* tiled_map)
{
    return tiled_map->tilesets;
}

static Sint32* get_layer_content(cute_tiled_layer_t* tiled_layer)
{
    return (Sint32*)tiled_layer->data;
}

static const char* get_layer_name(cute_tiled_layer_t* tiled_layer)
{
    return tiled_layer->name.ptr;
}

static Sint32 get_layer_property_count(cute_tiled_layer_t* tiled_layer)
{
    return tiled_layer->property_count;
}

static Sint32 get_local_id(Sint32 gid, cute_tiled_map_t* tiled_map)
{
    Sint32 local_id = gid - get_first_gid(tiled_map);
    return local_id >= 0 ? local_id : 0;
}

static Sint32 get_map_property_count(cute_tiled_map_t* tiled_map)
{
    return tiled_map->property_count;
}

static Sint32 get_next_animated_tile_id(Sint32 gid, Sint32 current_frame, cute_tiled_map_t* tiled_map)
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

static const int get_object_uid(cute_tiled_object_t* tiled_object)
{
    return tiled_object->id;
}

static const char* get_object_name(cute_tiled_object_t* tiled_object)
{
    return tiled_object->name.ptr;
}

static Sint32 get_object_property_count(cute_tiled_object_t* tiled_object)
{
    return tiled_object->property_count;
}

static const char* get_object_type_name(cute_tiled_object_t* tiled_object)
{
    return tiled_object->type.ptr;
}

static void get_tile_position(Sint32 gid, Sint32* pos_x, Sint32* pos_y, cute_tiled_map_t* tiled_map)
{
    cute_tiled_tileset_t* tileset  = tiled_map->tilesets;
    Sint32                local_id = get_local_id(gid, tiled_map);

    *pos_x = (local_id % tileset->columns) * get_tile_width(tiled_map);
    *pos_y = (local_id / tileset->columns) * get_tile_height(tiled_map);
}

static Sint32 get_tile_property_count(cute_tiled_tile_descriptor_t* tiled_tile)
{
    return tiled_tile->property_count;
}

static SDL_bool is_gid_valid(Sint32 gid, cute_tiled_map_t* tiled_map)
{
    if (gid)
    {
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

static SDL_bool is_tile_animated(Sint32 gid, Sint32* animation_length, Sint32* id, cute_tiled_map_t* tiled_map)
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

static Sint32 remove_gid_flip_bits(Sint32 gid)
{
    return cute_tiled_unset_flags(gid);
}

static SDL_bool tile_has_properties(Sint32 gid, cute_tiled_tile_descriptor_t** tile, cute_tiled_map_t* tiled_map)
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

static void load_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, ngine_t* core)
{
    int index = 0;

    for (index = 0; index < property_count; index += 1)
    {
        if (name_hash == generate_hash((const unsigned char*)properties[index].name.ptr))
        {
            break;
        }
    }
    // Entities are allowed to have no properties.
    if (0 == property_count)
    {
        return;
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
                core->map->integer_property = properties[index].data.integer;
                break;
            case CUTE_TILED_PROPERTY_BOOL:
                core->map->boolean_property = (SDL_bool)properties[index].data.boolean;
                break;
            case CUTE_TILED_PROPERTY_FLOAT:
                core->map->decimal_property = (float)properties[index].data.floating;
                break;
            case CUTE_TILED_PROPERTY_STRING:
                core->map->string_property  = properties[index].data.string.ptr;
                break;
        }
    }
}

static status_t create_and_set_render_target(SDL_Texture** target, ngine_t* core)
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
        // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return NG_ERROR;
    }

    if (0 > SDL_SetRenderTarget(core->renderer, (*target)))
    {
        // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        SDL_DestroyTexture((*target));
        return NG_ERROR;
    }

    SDL_RenderClear(core->renderer);

    return NG_OK;
}

Sint32 get_tile_index(Sint32 pos_x, Sint32 pos_y, ngine_t* core)
{
    Sint32 tile_index;

    tile_index  = pos_x  / get_tile_width(core->map->handle);
    tile_index += (pos_y / get_tile_height(core->map->handle)) * core->map->handle->width;

    if (tile_index > (core->map->tile_desc_count - 1))
    {
        tile_index = core->map->tile_desc_count - 1;
    }

    return tile_index;
}

void unload_tiled_map(ngine_t* core)
{
    core->map->hash_id_objectgroup = 0;
    core->map->hash_id_tilelayer   = 0;

    if (core->map->handle)
    {
        cute_tiled_free_map(core->map->handle);
    }
}

SDL_bool is_map_loaded(ngine_t* core)
{
    return core->is_map_loaded;
}

int get_tile_width(cute_tiled_map_t* tiled_map)
{
    return tiled_map->tilesets->tilewidth;
}

int get_tile_height(cute_tiled_map_t* tiled_map)
{
    return tiled_map->tilesets->tileheight;
}

SDL_bool get_boolean_map_property(const Uint64 name_hash, ngine_t* core)
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

float get_decimal_map_property(const Uint64 name_hash, ngine_t* core)
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

Sint32 get_integer_map_property(const Uint64 name_hash, ngine_t* core)
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

const char* get_string_map_property(const Uint64 name_hash, ngine_t* core)
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

SDL_bool get_boolean_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, ngine_t* core)
{
    core->map->boolean_property = SDL_FALSE;
    load_property(name_hash, properties, property_count, core);
    return core->map->boolean_property;
}

float get_decimal_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, ngine_t* core)
{
    core->map->decimal_property = 0.0;
    load_property(name_hash, properties, property_count, core);
    return core->map->decimal_property;
}

int32_t get_integer_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, ngine_t* core)
{
    core->map->integer_property = 0;
    load_property(name_hash, properties, property_count, core);
    return core->map->integer_property;
}

const char* get_string_property(const Uint64 name_hash, cute_tiled_property_t* properties, Sint32 property_count, ngine_t* core)
{
    core->map->string_property = NULL;
    load_property(name_hash, properties, property_count, core);
    return core->map->string_property;
}

void trigger_action(ngine_t* core)
{
    cute_tiled_layer_t*  layer;
    cute_tiled_object_t* tiled_object = NULL;

    if (! is_map_loaded(core))
    {
        // SDL_Log("No map has been loaded.");
        return;
    }

    layer = get_head_layer(core->map->handle);

    while (layer && core->map->entity_count)
    {
        if (is_tiled_layer_of_type(OBJECT_GROUP, layer, core))
        {
            Sint32 index = 0;
            tiled_object = get_head_object(layer, core);
            while (tiled_object)
            {
                cute_tiled_property_t* properties   = tiled_object->properties;
                Sint32                 prop_cnt     = get_object_property_count(tiled_object);
                entity_t*              entity_a     = &core->map->entity[index];
                entity_t*              entity_b     = &core->map->entity[core->map->active_entity - 1];
                Sint32                 tile_index_a = get_tile_index(entity_a->pos_x, entity_a->pos_y, core);
                Sint32                 tile_index_b = get_tile_index(entity_b->pos_x, entity_b->pos_y, core);

                if (tile_index_a == tile_index_b)
                {
                    if (get_string_property(H_display_text, properties, prop_cnt, core))
                    {
                        set_display_text(core->map->string_property, core);
                        break;
                    }
                }

                index        += 1;
                tiled_object = tiled_object->next;
            }
        }
        layer = layer->next;
    }
}

status_t load_font(ngine_t* core)
{
    status_t status = NG_OK;

    if (NG_OK != load_texture_from_file((const char*)"font.bmp", &core->font_texture, core))
    {
        // SDL_Log("%s: Error loading image '%s'.", FUNCTION_NAME, tileset_file_name);
        status = NG_ERROR;
    }

    clear_display_text(core);

warning:
    return status;
}

status_t load_tiles(ngine_t* core)
{
    cute_tiled_layer_t* layer = get_head_layer(core->map->handle);

    core->map->tile_desc_count = (Sint32)(core->map->handle->height * core->map->handle->width);

    if (core->map->tile_desc_count < 0)
    {
        return NG_OK;
    }

    core->map->tile_desc = (tile_desc_t*)calloc((size_t)core->map->tile_desc_count, sizeof(struct tile_desc));
    if (! core->map->tile_desc)
    {
        // SDL_Log("%s: error allocating memory.", FUNCTION_NAME);
        return NG_ERROR;
    }

    while (layer)
    {
        if (is_tiled_layer_of_type(TILE_LAYER, layer, core))
        {
            Sint32 index_height;
            Sint32 index_width;
            for (index_height = 0; index_height < (Sint32)core->map->handle->height; index_height += 1)
            {
                for (index_width = 0; index_width < (Sint32)core->map->handle->width; index_width += 1)
                {
                    cute_tiled_tileset_t*         tileset       = get_head_tileset(core->map->handle);
                    cute_tiled_tile_descriptor_t* tile          = tileset->tiles;
                    Sint32*                       layer_content = get_layer_content(layer);
                    Sint32                        tile_index    = (index_height * (Sint32)core->map->handle->width) + index_width;
                    Sint32                        gid           = remove_gid_flip_bits((Sint32)layer_content[tile_index]);

                    if (tile_has_properties(gid, &tile, core->map->handle))
                    {
                        Sint32 prop_cnt = get_tile_property_count(tile);

                        if (get_boolean_property(H_is_solid, tile->properties, prop_cnt, core))
                        {
                            core->map->tile_desc[tile_index].is_solid = SDL_TRUE;
                        }
                    }
                }
            }
        }
        layer = layer->next;
    }

    return NG_OK;
}

status_t load_tileset(ngine_t* core)
{
    status_t status                = NG_OK;
    char     tileset_file_name[16] = { 0 };

    stbsp_snprintf(tileset_file_name, 16, "%s", core->map->handle->tilesets->image.ptr);

    if (NG_OK != load_texture_from_file((const char*)tileset_file_name, &core->map->tileset_texture, core))
    {
        // SDL_Log("%s: Error loading image '%s'.", FUNCTION_NAME, tileset_file_name);
        status = NG_ERROR;
    }

warning:
    return status;
}

status_t load_tiled_map(const char* map_file_name, ngine_t* core)
{
    cute_tiled_layer_t* layer;
    Uint8*              resource_buf;

    resource_buf = (Uint8*)load_binary_file_from_path(map_file_name);
    if (! resource_buf)
    {
        // SDL_Log("Failed to load resource: %s", map_file_name);
        return NG_ERROR;
    }

    core->map->handle = cute_tiled_load_map_from_memory((const void*)resource_buf, size_of_file(map_file_name), NULL);
    if (! core->map->handle)
    {
        free(resource_buf);
        // SDL_Log("%s: %s.", FUNCTION_NAME, cute_tiled_error_reason);
        return NG_WARNING;
    }
    free(resource_buf);

    layer = get_head_layer(core->map->handle);
    while (layer)
    {
        if (H_tilelayer == generate_hash((const unsigned char*)layer->type.ptr) && !core->map->hash_id_tilelayer)
        {
            core->map->hash_id_tilelayer = layer->type.hash_id;
            // SDL_Log("Set hash ID for tile layer: %llu", core->map->hash_id_tilelayer);
        }
        else if (H_objectgroup == generate_hash((const unsigned char*)layer->type.ptr) && !core->map->hash_id_objectgroup)
        {
            core->map->hash_id_objectgroup = layer->type.hash_id;
            // SDL_Log("Set hash ID for object group: %llu", core->map->hash_id_objectgroup);
        }
        layer = layer->next;
    }

    return NG_OK;
}

status_t load_animated_tiles(ngine_t* core)
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
        return NG_OK;
    }
    else
    {
        core->map->animated_tile = (animated_tile_t*)calloc((size_t)animated_tile_count, sizeof(struct animated_tile));
        if (! core->map->animated_tile)
        {
            // SDL_Log("%s: error allocating memory.", FUNCTION_NAME);
            return NG_ERROR;
        }
    }

    // SDL_Log("Load %u animated tile(s).", animated_tile_count);

    return NG_OK;
}

status_t load_sprites(ngine_t* core)
{
    status_t status            = NG_OK;
    char     property_name[17] = { 0 };
    SDL_bool search_is_running = SDL_TRUE;
    Sint32   prop_cnt          = get_map_property_count(core->map->handle);
    Sint32   index;

    core->map->sprite_count = 0;

    while (search_is_running)
    {
        stbsp_snprintf(property_name, 17, "sprite_sheet_%u", core->map->sprite_count + 1);

        if (get_string_property(generate_hash((const unsigned char*)property_name), core->map->handle->properties, prop_cnt, core))
        {
            core->map->sprite_count += 1;
        }
        else
        {
            search_is_running = SDL_FALSE;
        }
    }

    if (0 == core->map->sprite_count)
    {
        return NG_OK;
    }

    core->map->sprite = (sprite_t*)calloc((size_t)core->map->sprite_count, sizeof(struct sprite));
    if (! core->map->sprite)
    {
        // SDL_Log("%s: error allocating memory.", FUNCTION_NAME);
        return NG_ERROR;
    }

    for (index = 0; index < core->map->sprite_count; index += 1)
    {
        const char* file_name;
        stbsp_snprintf(property_name, 17, "sprite_sheet_%u", index + 1);

        file_name = get_string_property(generate_hash((const unsigned char*)property_name), core->map->handle->properties, prop_cnt, core);

        if (file_name)
        {
            Sint32 source_length       = (Sint32)(strlen(file_name) + 1);
            char*  sprite_image_source = (char*)calloc(1, source_length);
            if (! sprite_image_source)
            {
                // SDL_Log("%s: error allocating memory.", FUNCTION_NAME);
                return NG_ERROR;
            }

            stbsp_snprintf(sprite_image_source, source_length, "%s", file_name);

            core->map->sprite[index].id = index + 1;

            status = load_texture_from_file(sprite_image_source, &core->map->sprite[index].texture, core);
            if (NG_OK != status)
            {
                free(sprite_image_source);
                return status;
            }
        }
    }

    return status;
}

status_t load_entities(ngine_t* core)
{
    cute_tiled_layer_t*  layer        = get_head_layer(core->map->handle);
    cute_tiled_object_t* tiled_object = NULL;

    if (core->map->entity_count)
    {
        /* Nothing else to do here. */
        return NG_OK;
    }

    while (layer)
    {
        if (is_tiled_layer_of_type(OBJECT_GROUP, layer, core))
        {
            tiled_object = get_head_object(layer, core);
            while (tiled_object)
            {
                core->map->entity_count += 1;
                tiled_object            = tiled_object->next;
            }
        }
        layer = layer->next;
    }

    if (core->map->entity_count)
    {
        core->map->entity = (entity_t*)calloc((size_t)core->map->entity_count, sizeof(struct entity));
        if (! core->map->entity)
        {
            // SDL_Log("%s: error allocating memory.", FUNCTION_NAME);
            return NG_ERROR;
        }
    }

    // SDL_Log("Load %u entities:", core->map->entity_count);

    layer = get_head_layer(core->map->handle);
    while (layer)
    {
        if (is_tiled_layer_of_type(OBJECT_GROUP, layer, core))
        {
            Sint32 index = 0;
            tiled_object = get_head_object(layer, core);
            while (tiled_object)
            {
                entity_t*              entity     = &core->map->entity[index];
                cute_tiled_property_t* properties = tiled_object->properties;
                Sint32                 prop_cnt   = get_object_property_count(tiled_object);

                entity->handle                = tiled_object;
                entity->pos_x                 = (Sint32)tiled_object->x;
                entity->pos_y                 = (Sint32)tiled_object->y;
                entity->uid                   = (Sint32)get_object_uid(tiled_object);
                entity->id                    = (Sint32)index + 1;
                entity->width                 = (Sint32)get_integer_property(H_width,     properties, prop_cnt, core);
                entity->height                = (Sint32)get_integer_property(H_height,    properties, prop_cnt, core);
                entity->sprite_id             = (Sint32)get_integer_property(H_sprite_id, properties, prop_cnt, core);
                entity->show_animation        = (SDL_bool)SDL_FALSE;
                entity->animation.first_frame = (Sint32)1;
                entity->animation.fps         = (Sint32)0;
                entity->animation.length      = (Sint32)0;
                entity->animation.offset_y    = (Sint32)1;

                if (entity->width <= 0)
                {
                    entity->width = get_tile_width(core->map->handle);
                }

                if (entity->height <= 0)
                {
                    entity->width = get_tile_height(core->map->handle);
                }

                if (get_boolean_property(H_is_player, properties, prop_cnt, core))
                {
                    core->map->active_entity = entity->id;
                    core->camera.is_locked   = SDL_TRUE;
                }

                index        += 1;
                tiled_object  = tiled_object->next;
            }
        }
        layer = layer->next;
    }

    return NG_OK;
}

status_t render_scene(ngine_t* core)
{
    cute_tiled_layer_t* layer;
    Sint32              index;

    if (! core->is_map_loaded)
    {
        return NG_OK;
    }

    if (NG_OK != create_and_set_render_target(&core->render_target, core))
    {
        return NG_ERROR;
    }

    // Update and render animated tiles.
    core->map->time_since_last_anim_frame += core->time_since_last_frame;

    if (0 < core->map->animated_tile_index && core->map->time_since_last_anim_frame >= (Uint32)(1000 / ANIM_TILE_FPS))
    {
        core->map->time_since_last_anim_frame = 0;

        if (0 > SDL_SetRenderTarget(core->renderer, core->map->layer_texture))
        {
            // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return NG_ERROR;
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
                // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
                return NG_ERROR;
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

    // Texture has already been rendered.
    if (core->map->layer_texture)
    {
        Sint32   render_pos_x = 0 - core->camera.pos_x;
        Sint32   render_pos_y = 0 - core->camera.pos_y;
        SDL_Rect dst          = {
            (Sint32)render_pos_x,
            (Sint32)render_pos_y,
            (Sint32)core->map->width,
            (Sint32)core->map->height
        };

        if (0 > SDL_RenderCopyEx(core->renderer, core->map->layer_texture, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
        {
            // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return NG_ERROR;
        }

        // Update and render entity entities.
        index = 0;
        while (layer && core->map->entity_count)
        {
            if (is_tiled_layer_of_type(OBJECT_GROUP, layer, core))
            {
                cute_tiled_object_t* tiled_object = get_head_object(layer, core);
                while (tiled_object)
                {
                    entity_t* entity = &core->map->entity[index];
                    Sint32   pos_x = entity->pos_x - core->camera.pos_x;
                    Sint32   pos_y = entity->pos_y - core->camera.pos_y;
                    SDL_Rect dst   = { 0 };
                    SDL_Rect src   = { 0 };

                    if (entity->show_animation)
                    {
                        entity->animation.time_since_last_anim_frame += core->time_since_last_frame;
                    }

                    src.x  = (entity->animation.first_frame - 1) * entity->width;
                    src.y  = entity->animation.offset_y          * entity->height;

                    if (entity->show_animation && !core->display_text)
                    {
                        entity->animation.time_since_last_anim_frame += core->time_since_last_frame;

                        if (entity->animation.time_since_last_anim_frame >= (Uint32)(1000 / entity->animation.fps))
                        {
                            entity->animation.time_since_last_anim_frame  = 0;
                            entity->animation.current_frame              += 1;

                            if (entity->animation.current_frame >= entity->animation.length)
                            {
                                entity->animation.current_frame = 0;
                            }
                        }

                        src.x += entity->animation.current_frame * entity->width;
                    }
                    else
                    {
                        entity->animation.current_frame = entity->animation.first_frame;
                    }

                    src.w  = entity->width;
                    src.h  = entity->height;
                    dst.x  = (Sint32)pos_x - (entity->width  / 2);
                    dst.y  = (Sint32)pos_y - (entity->height / 2);
                    dst.w  = entity->width;
                    dst.h  = entity->height;

                    // We do not need to draw entities that are not
                    // inside the viewport.
                    if ((dst.x > (0 - entity->width)) && (dst.x < 176))
                    {
                        if ((dst.y > (0 - entity->height)) && (dst.y < 208))
                        {
                            // We do not draw entities that have no
                            // sprite either and if the sprite requested
                            // does not exist, there is also nothing to
                            // do here.
                            if ((entity->sprite_id > 0) && &core->map->sprite[entity->sprite_id - 1])
                            {
                                if (core->map->sprite[entity->sprite_id - 1].texture)
                                {
                                    if (0 > SDL_RenderCopyEx(core->renderer, core->map->sprite[entity->sprite_id - 1].texture, &src, &dst, 0, NULL, SDL_FLIP_NONE))
                                    {
                                        // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
                                        return NG_ERROR;
                                    }
                                }
                            }
                            if (core->debug_mode)
                            {
                                SDL_Rect tile_frame;
                                Sint32   tile_index;

                                tile_index = get_tile_index(entity->pos_x, entity->pos_y, core);

                                tile_frame.w = get_tile_width(core->map->handle);
                                tile_frame.h = get_tile_height(core->map->handle);
                                tile_frame.x = (tile_index % core->map->handle->width) * tile_frame.w;
                                tile_frame.y = (tile_index / core->map->handle->width) * tile_frame.h;

                                tile_frame.x = tile_frame.x - core->camera.pos_x;
                                tile_frame.y = tile_frame.y - core->camera.pos_y;

                                if (core->map->tile_desc[tile_index].is_solid)
                                {
                                    SDL_SetRenderDrawColor(core->renderer, 0xff, 0x00, 0x00, 0x00);
                                }
                                else
                                {
                                    SDL_SetRenderDrawColor(core->renderer, 0x00, 0xff, 0x00, 0x00);
                                }
                                SDL_RenderDrawRect(core->renderer, &tile_frame);
                            }
                        }
                    }
                    index        += 1;
                    tiled_object  = tiled_object->next;
                }
            }
            layer = layer->next;
        }

        if (core->display_text)
        {
            render_text(core);
        }

        return NG_OK;
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
        // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return NG_ERROR;
    }

    if (0 > SDL_SetRenderTarget(core->renderer, core->map->layer_texture))
    {
        // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return NG_ERROR;
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
                    // SDL_Log("Render map layer: %s", layer_name);
                }
            }
        }
        layer = layer->next;
    }

    if (0 > SDL_SetRenderTarget(core->renderer, core->render_target))
    {
        // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return NG_ERROR;
    }

    return NG_OK;
}

// https://github.com/ngagesdk/nrpg/issues/2
status_t render_scene_ex(ngine_t* core)
{
    status_t status = NG_OK;

    if (! core->is_map_loaded)
    {
        return NG_OK;
    }

    if (! core->map->layer_texture)
    {
        core->map->layer_texture = SDL_CreateTexture(
            core->renderer,
            SDL_PIXELFORMAT_RGB444,
            SDL_TEXTUREACCESS_TARGET,
            256, 256);

        if (! core->map->layer_texture)
        {
            // SDL_Log("%s: %s.", FUNCTION_NAME, SDL_GetError());
            status = NG_ERROR;
            goto exit;
        }
    }

    // tbd.

exit:
    return status;
}
