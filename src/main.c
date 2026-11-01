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
  SDL_Window *window;
  SDL_Renderer *renderer;
  bool running;
} Game;


bool start(Game *g)
{
  if (!SDL_Init(SDL_INIT_VIDEO)) return false;

  if (!SDL_CreateWindowAndRenderer("First tilemap", 960, 570, SDL_WINDOW_RESIZABLE, &g->window, &g->renderer))
    return false;

  if (!SDL_SetRenderLogicalPresentation(g->renderer, MAP_W, MAP_H, SDL_LOGICAL_PRESENTATION_LETTERBOX))
    return false;

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
}

void stop(Game *g)
{
  SDL_DestroyRenderer(g->renderer);
  SDL_DestroyWindow(g->window);
  SDL_Quit();
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
