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
    Sint32  pos_x;
    Sint32  pos_y;
    Sint32  max_pos_x;
    Sint32  max_pos_y;

} camera_t;

typedef struct animated_tile
{
    Sint32 dst_x;
    Sint32 dst_y;
    Sint32 animation_length;
    Sint32 current_frame;
    Sint32 gid;
    Sint32 id;

} animated_tile_t;

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
    Uint32*            tile_properties;

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
    Uint32        time_since_last_frame;
    Uint32        time_a;
    Uint32        time_b;

} core_t;

status_t init_core(const char* title, core_t** core);
status_t update_core(core_t* core);
void     free_core(core_t *core);
status_t load_map(const char* map_file_name, const char* tileset_file_name, core_t* core);
void     unload_map(core_t* core);

#endif /* CORE_H */
