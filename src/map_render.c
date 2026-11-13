
#include "atlas.h"
#include "tiled_map.h"
#include <map_render.h>
#include <config.h>
#include <stddef.h>


static void cell_draw(Game *g, const int *cells)
{
  for(int y = 0; y < g->map.height; ++y)
  {
    for(int x = 0; x < g->map.width; ++x)
    {
      int gid = get_map_gid(&g->map, cells, x, y);
      tile_draw(g, gid, x*CELL, y*CELL);
    }
  }
}

void map_render(Game *g)
{
  cell_draw(g, g->map.ground);

  // draw the layers
  for (size_t i = 0; i < g->map.layer_count; ++i)
  {
    const MapLayer *l = &g->map.layers[i];
    if (!l->visible) continue;

    SDL_SetTextureAlphaModFloat(g->atlas, l->opacity);
    cell_draw(g, l->cells);
  }

  SDL_SetTextureAlphaModFloat(g->atlas, 1);
}
