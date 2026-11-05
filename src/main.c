#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAP_W 640
#define MAP_H 360
#define CELL 32
#define NS_PER_SECOND 1e9f

typedef struct {
  SDL_Texture *atlas;
  int rows, columns, tile_size;
} MapAtlas;

typedef struct {
  int cells[MAP_H][MAP_W];
} Map;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  bool running;
  Map map;
  MapAtlas map_atlas;
} Game;

void map_draw(Game *g);

bool map_read(Map *out, const char *path)
{
  FILE *f = fopen(path, "r");
  if (!f) {
    SDL_SetError("File not found : %s\n", path);
    return false;
  }

  Map next = {0};
  int width, height = 0;
  bool ok = fscanf(f, "%d %d", &width, &height) == 2;
  ok = ok && width == 32 && height == 24;

  for (int y = 0; ok && y < 32; ++y)
  {
    for (int x = 0; ok && x < 24; ++x)
    {
      int id = 0;
      ok = fscanf(f, "%d", &id) == 1 && id >= 0 && id <= 64;
      if (ok) next.cells[y][x] = id;
    }
  }

  if (fclose(f) != 0) ok = false;

  if (ok) *out = next;

  return ok;
}

bool start(Game *g)
{
  if (!SDL_Init(SDL_INIT_VIDEO)) return false;

  if (!SDL_CreateWindowAndRenderer("First tilemap", 960, 570, SDL_WINDOW_RESIZABLE, &g->window, &g->renderer))
    return false;

  if (!SDL_SetRenderLogicalPresentation(g->renderer, MAP_W, MAP_H, SDL_LOGICAL_PRESENTATION_LETTERBOX))
    return false;


  g->map_atlas.atlas = IMG_LoadTexture(g->renderer, "assets/world_tileset.png");
  if (!g->map_atlas.atlas) return false;

  SDL_SetTextureScaleMode(g->map_atlas.atlas, SDL_SCALEMODE_PIXELART);
  g->map_atlas.rows = 8;
  g->map_atlas.columns = 8;
  g->map_atlas.tile_size = 16;

  if (!map_read(&g->map, "maps/start.map"))
  {
    return false;
  }

  g->running = true;
  return true;
}

void update(Game *g, float dt)
{
  (void)g; (void)dt;
}

void draw(Game *g)
{
  SDL_SetRenderDrawColor(g->renderer, 0, 43, 58, 255);
  SDL_RenderClear(g->renderer);
  // map rendering
  map_draw(g);
}

void stop(Game *g)
{
  SDL_DestroyRenderer(g->renderer);
  SDL_DestroyWindow(g->window);
  SDL_Quit();
}

void rectangle(Game *g, int x, int y, int w, int h, SDL_Color c)
{
  SDL_SetRenderDrawColor(g->renderer, c.r, c.g, c.b, c.a);

  SDL_FRect box = {x, y, w, h};
  SDL_RenderFillRect(g->renderer, &box);
}

void tile(Game *g, int x, int y, int id)
{
  int tile_size = g->map_atlas.tile_size;
  int tx = id % g->map_atlas.columns;
  int ty = id / g->map_atlas.rows;

  SDL_FRect source = {tx*tile_size, ty*tile_size, tile_size, tile_size};
  SDL_FRect dest = {x, y, CELL, CELL};
  SDL_RenderTexture(g->renderer, g->map_atlas.atlas, &source, &dest);
}

void map_draw(Game *g)
{
  for(int y = 0; y < MAP_H; ++y)
  {
    for(int x = 0; x < MAP_W; ++x)
    {
      int id = g->map.cells[y][x];
      tile(g, x*CELL, y*CELL, id);
    }
  }
}

int main(void) {
  Game g = {0};
  if (!start(&g)) {
    fprintf(stderr, "Startup error: %s \n", SDL_GetError());
    return EXIT_FAILURE;
  }

  Uint64 previous = SDL_GetTicksNS();
  while (g.running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_EVENT_QUIT) g.running = false;
    }

    Uint64 now = SDL_GetTicksNS();
    float dt = (float)(now-previous) / NS_PER_SECOND;
    if (dt > 0.05f) dt = 0.05f;

    update(&g, dt);
    draw(&g);

    SDL_RenderPresent(g.renderer);
    SDL_Delay(16);
  }

  stop(&g);

  return EXIT_SUCCESS;
}
