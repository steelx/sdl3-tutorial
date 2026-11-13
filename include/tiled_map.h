#ifndef SDL_RPG_TILED_MAP_H
#define SDL_RPG_TILED_MAP_H

#include <stdbool.h>
#include <stddef.h>

#define MAP_MAX_W 32
#define MAP_MAX_H 24

// may layers
typedef struct {
  int id;
  char name[32];
  int *cells;
  bool visible;
  float opacity;
} MapLayer;

typedef struct {
  int *ground;
  bool *collsions;
  MapLayer *layers;
  size_t layer_count;

  int width, height;
  int tile_width, tile_height;
  char image[512];
  int firstgid, columns, tile_count, image_width, image_height;
} Map;

bool map_read(Map *out, const char *path);
void map_free(Map *m);
int get_map_gid(const Map *m, const int *cells, int x, int y);
bool get_collision_gid(const Map *m, int x, int y);

#endif /* SDL_RPG_TILED_MAP_H */
