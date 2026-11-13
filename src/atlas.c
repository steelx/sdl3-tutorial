
#include <atlas.h>
#include <config.h>

void tile_draw(Game *g, int gid, int x, int y)
{
  if ((g->map.firstgid == 0 && gid < 0) || (g->map.firstgid == 1 && gid <= 0)) {
    if (gid < 0) fprintf(stderr, "Error atlas.c: invalid GID: %d \n", gid);
    return;
  }
  int idx = gid - g->map.firstgid;
  int tx = idx % g->map.columns;
  int ty = idx / g->map.columns;

  SDL_FRect source = {
    tx*g->map.tile_width, ty*g->map.tile_height,
    g->map.tile_width, g->map.tile_height
  };

  SDL_FRect dest = {x, y, CELL, CELL};
  SDL_RenderTexture(g->renderer, g->atlas, &source, &dest);
}

