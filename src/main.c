#include "map_render.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <tiled_map.h>
#include <game.h>
#include <config.h>

bool map_texture(Game *g, const Map *m);


bool start(Game *g)
{
  if (!SDL_Init(SDL_INIT_VIDEO)) return false;

  if (!SDL_CreateWindowAndRenderer("First tilemap", WINDOW_W, WINDOW_H, SDL_WINDOW_RESIZABLE, &g->window, &g->renderer))
    return false;

  if (!SDL_SetRenderLogicalPresentation(g->renderer, VIEW_W, VIEW_H, SDL_LOGICAL_PRESENTATION_LETTERBOX))
    return false;


  if (!map_read(&g->map, "maps/small_room.json")) return false;
  if (!map_texture(g, &g->map)) return false;

  printf("map=%dx%d tile=%dx%d\n", g->map.width, g->map.height, g->map.tile_width, g->map.tile_height);
  printf("atlas: %s, tile_count=%d, image: %dx%d\n", g->map.image, g->map.tile_count, g->map.image_width, g->map.image_height);
  printf("ground[10,1]=%d\n", get_map_gid(&g->map, g->map.ground, 10, 1));
  printf("collision[10,1]=%d\n", get_collision_gid(&g->map, 10, 1));
  printf("visual layers=%zu\n",g->map.layer_count);

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
  map_render(g);
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

bool map_texture(Game *g, const Map *m)
{
  SDL_Texture *next = IMG_LoadTexture(g->renderer, m->image);
  if (!next) return false;

  float w, h = 0;
  if (!SDL_GetTextureSize(next, &w, &h) || w != m->image_width || h != m->image_height)
  {
    SDL_DestroyTexture(next);
    return false;
  }

  SDL_SetTextureScaleMode(next, SDL_SCALEMODE_PIXELART);
  SDL_DestroyTexture(g->atlas);
  g->atlas = next;
  return true;
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
