// Spdx-License-Identifier: MIT

#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>

typedef struct aabb
{
    Uint8 bottom;
    Uint8 left;
    Uint8 right;
    Uint8 top;

} aabb_t;

SDL_bool bb_do_intersect(const aabb_t bb_a, const aabb_t bb_b);
Uint64   generate_hash(const unsigned char* name);

#endif /* UTILS_H */
