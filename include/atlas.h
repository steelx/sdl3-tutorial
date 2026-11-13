#ifndef SDL_RPG_ATLAS_H
#define SDL_RPG_ATLAS_H

#include "game.h"

// tile_draw: atlas owns the GID to image.
void tile_draw(Game *g, int gid, int x, int y);

#endif /* SDL_RPG_ATLAS_H */
