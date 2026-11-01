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
  int cells[MAP_H][MAP_W];
} Map;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  bool running;
  Map map;
} Game;

void map_draw(Game *g);

bool start(Game *g)
{
  if (!SDL_Init(SDL_INIT_VIDEO)) return false;

  if (!SDL_CreateWindowAndRenderer("First tilemap", 960, 570, SDL_WINDOW_RESIZABLE, &g->window, &g->renderer))
    return false;

  if (!SDL_SetRenderLogicalPresentation(g->renderer, MAP_W, MAP_H, SDL_LOGICAL_PRESENTATION_LETTERBOX))
    return false;

  for(int y = 0; y < MAP_H; ++y)
  {
    for(int x = 0; x < MAP_W; ++x)
    {
      int id = (x == 0 || y == 0 || x == MAP_W-1 || y == MAP_H-1) ? 1 : 2;
      g->map.cells[y][x] = id;
    }
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

void map_draw(Game *g)
{
  for(int y = 0; y < MAP_H; ++y)
  {
    for(int x = 0; x < MAP_W; ++x)
    {
      SDL_Color colour = g->map.cells[y][x] == 1 ? (SDL_Color){255, 0, 0, 255} : (SDL_Color){0, 150, 1, 255};
      rectangle(g, x*CELL, y*CELL, CELL-1, CELL-1, colour);
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
