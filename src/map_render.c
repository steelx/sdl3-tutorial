
#include "atlas.h"
#include "tiled_map.h"
#include <map_render.h>
#include <config.h>
#include <stddef.h>

static void cell_draw(Game *g, const int *cells);
static void props_draw(Game *g);
static void debug_draw(Game *g);


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

  props_draw(g);
  SDL_SetTextureAlphaModFloat(g->atlas, 1);
  if (g->debug) debug_draw(g);
}


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


static void props_draw(Game *g)
{
  for (size_t i = 0; i < g->map.prop_count; ++i)
  {
    const MapProp *prop = &g->map.props[i];
    tile_draw(g, prop->gid, prop->x*SCALE, (prop->y - prop->height)*SCALE);
  }
}

static void debug_draw(Game *g)
{
  // draw debug spawn markers
  for (size_t i = 0; i < g->map.spawn_count; ++i)
  {
    const MapSpawn *s = &g->map.spawns[i];

    const float inset = s->point ? 10 : 2;
    SDL_FRect mark = {
      (s->tx*CELL)+inset, (s->ty*CELL)+inset,
      CELL-inset, CELL-inset
    };
    SDL_Color c = s->point ? (SDL_Color){255,0,0,255} : (SDL_Color){0,255,0,255};
    SDL_SetRenderDrawColor(g->renderer, c.r, c.g, c.b, c.a);

    SDL_RenderRect(g->renderer, &mark);
  }
}
