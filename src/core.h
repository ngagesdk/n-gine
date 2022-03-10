// Spdx-License-Identifier: MIT

#ifndef CORE_H
#define CORE_H

#include <SDL.h>
#include "cute_tiled.h"

#ifndef FUNCTION_NAME
#  if defined(__NGAGE__)
#    define FUNCTION_NAME __FUNCTION__
#  else
#    define FUNCTION_NAME __func__
#  endif
#endif

#define H_objectgroup 0xc0b9d518970be349
#define H_tilelayer   0x0377d9f70e844fb0
#define H_width       0x0000003110a3b0a5
#define H_height      0x0000065301d688de
#define H_sprite_id   0x0377d8f6e7994748
#define H_is_solid    0x001ae728dd16b21b

typedef enum status
{
    CORE_OK = 0,
    CORE_WARNING,
    CORE_ERROR,
    CORE_EXIT

} status_t;

typedef enum
{
    TILE_LAYER = 0,
    OBJECT_GROUP

} tiled_layer_type;

typedef struct camera
{
    Sint32   pos_x;
    Sint32   pos_y;
    Sint32   max_pos_x;
    Sint32   max_pos_y;
    Sint32   target_actor_id;
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

typedef struct aabb
{
    Uint8 bottom;
    Uint8 left;
    Uint8 right;
    Uint8 top;

} aabb_t;

typedef struct actor
{
    Sint32               pos_x;
    Sint32               pos_y;
    cute_tiled_object_t* handle;
    Sint32               id;
    Sint32               index;
    Sint32               width;
    Sint32               height;
    Sint32               sprite_id;
    SDL_bool             show_animation;
    animation_t          animation;

} actor_t;

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
    Sint32             pos_x;
    Sint32             pos_y;

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

    actor_t*           actor;
    Sint32             actor_count;
    sprite_t*          sprite;
    Sint32             sprite_count;
    tile_desc_t*       tile_desc;
    Sint32             tile_desc_count;

} map_t;

typedef struct core
{
    SDL_Renderer* renderer;
    SDL_Texture*  render_target;
    SDL_Window*   window;
    map_t*        map;
    struct camera camera;
    SDL_bool      is_active;
    SDL_bool      is_map_loaded;
    SDL_bool      debug_mode;
    Uint32        time_since_last_frame;
    Uint32        time_a;
    Uint32        time_b;

} core_t;

status_t init_core(const char* resource_file, const char* title, core_t** core);
status_t update_core(core_t* core);
void     free_core(core_t *core);
status_t load_map(const char* map_name, core_t* core);
void     unload_map(core_t* core);

#endif /* CORE_H */
