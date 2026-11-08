#ifndef SDL_RPG_TILED_MAP_H
#define SDL_RPG_TILED_MAP_H

#include <stdbool.h>

#define MAP_MAX_W 32
#define MAP_MAX_H 24


typedef struct {
  int width, height;
  int tile_width, tile_height;
  char image[512];
  int firstgid, columns, tile_count, image_width, image_height;
} Map;

bool map_read(Map *out, const char *path);

#endif /* SDL_RPG_TILED_MAP_H */
