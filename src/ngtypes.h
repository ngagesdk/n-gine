/** @file ngtypes.h
 *
 *  N-GINE, a portable game engine which is being developed specifically
 *  for the Nokia N-Gage.
 *
 *  Internal type definitions.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef NGTYPES_H
#define NGTYPES_H

#include <SDL.h>
#include <cute_tiled.h>

typedef enum status
{
    NG_OK = 0,
    NG_WARNING,
    NG_ERROR,
    NG_EXIT

} status_t;

typedef enum
{
    TILE_LAYER = 0,
    OBJECT_GROUP

} tiled_layer_type;

typedef struct aabb
{
    Uint8 bottom;
    Uint8 left;
    Uint8 right;
    Uint8 top;

} aabb_t;

typedef struct camera
{
    Sint32   pos_x;
    Sint32   pos_y;
    Sint32   max_pos_x;
    Sint32   max_pos_y;
    SDL_bool is_locked;

} camera_t;

typedef struct animation
{
    Uint32 time_since_last_anim_frame;
    Sint32 current_frame;
    Sint32 first_frame;
    Sint32 fps;
    Sint32 length;
    Sint32 offset_y;

} animation_t;

typedef struct entity
{
    cute_tiled_object_t* handle;
    Sint32               pos_x;
    Sint32               pos_y;
    Sint32               uid;
    Sint32               id;
    Sint32               index;
    Sint32               width;
    Sint32               height;
    Sint32               sprite_id;
    SDL_bool             show_animation;
    animation_t          animation;

} entity_t;

typedef struct sprite
{
    SDL_Texture* texture;
    Sint32       id;

} sprite_t;

typedef struct animated_tile
{
    Sint32 dst_x;
    Sint32 dst_y;
    Sint32 animation_length;
    Sint32 current_frame;
    Sint32 gid;
    Sint32 id;

} animated_tile_t;

typedef struct tile_desc
{
    SDL_bool is_solid;

} tile_desc_t;

typedef struct map
{
    cute_tiled_map_t*  handle;
    long long unsigned hash_id_objectgroup;
    long long unsigned hash_id_tilelayer;

    Sint32             width;
    Sint32             height;

    animated_tile_t*   animated_tile;
    Sint32             animated_tile_index;
    Uint32             time_since_last_anim_frame;

    SDL_Texture*       animated_tile_texture;
    SDL_Texture*       layer_texture;
    SDL_Texture*       tileset_texture;

    SDL_bool           boolean_property;
    float              decimal_property;
    Sint32             integer_property;
    const char*        string_property;

    entity_t*          entity;
    Sint32             entity_count;
    Sint32             active_entity;
    sprite_t*          sprite;
    Sint32             sprite_count;
    tile_desc_t*       tile_desc;
    Sint32             tile_desc_count;

} map_t;

typedef struct ngine
{
    SDL_Renderer*  renderer;
    SDL_Texture*   render_target;
    SDL_Texture*   font_texture;
    unsigned char* display_text;
    SDL_Window*    window;
    map_t*         map;
    struct camera  camera;
    SDL_bool       is_map_loaded;
    SDL_bool       debug_mode;
    Uint32         time_since_last_frame;
    Uint32         time_a;
    Uint32         time_b;

} ngine_t;

#endif /* NGTYPES_H */
