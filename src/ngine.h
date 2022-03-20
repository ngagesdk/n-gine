// Spdx-License-Identifier: MIT

#ifndef NGINE_H
#define NGINE_H

#include <SDL.h>
#include "cute_tiled.h"
#include "types.h"

#ifndef FUNCTION_NAME
#  if defined(__NGAGE__)
#    define FUNCTION_NAME __FUNCTION__
#  else
#    define FUNCTION_NAME __func__
#  endif
#endif

#define H_objectgroup    0xc0b9d518970be349
#define H_tilelayer      0x0377d9f70e844fb0
#define H_width          0x0000003110a3b0a5
#define H_height         0x0000065301d688de
#define H_sprite_id      0x0377d8f6e7994748
#define H_is_solid       0x001ae728dd16b21b
#define H_is_player      0x0377cc4478b16e8d
#define H_map_right      0x0377d0b4a3693ac0
#define H_map_left       0x001ae74b4ac1c56d
#define H_map_up         0x000006530d3ba847
#define H_map_down       0x001ae74b4abd8f1a
#define H_display_text   0xd064eba5e9b9b1df
// Could be fun if the engine supports platformer games.
#define H_meter_in_pixel 0xfbbc8a6d4a407cf9
#define H_gravity        0x0000d0b30d77f26b

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

status_t ng_init_core(const char* resource_file, const char* title, ngine_t** core);
status_t ng_update_core(ngine_t* core);
void     ng_free_core(ngine_t *core);
status_t ng_load_map(const char* map_name, ngine_t* core);
void     ng_unload_map(ngine_t* core);

#endif /* NGINE_H */
