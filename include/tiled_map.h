#ifndef SDL_RPG_TILED_MAP_H
#define SDL_RPG_TILED_MAP_H

#include <stdbool.h>
#include <stddef.h>

#define MAP_MAX_W 32
#define MAP_MAX_H 24

typedef struct {
  int gid;
  char name[32];
  int x, y, width, height;
} MapProp;

typedef struct {
  char name[32];
  int tx, ty; /* tile */
  bool point;/* marker is a point, false = rectangle */
} MapSpawn;

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
  MapLayer *layers; size_t layer_count;
  MapProp *props; size_t prop_count;
  MapSpawn *spawns; size_t spawn_count;

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
