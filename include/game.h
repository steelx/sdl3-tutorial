#ifndef SDL_RPG_GAME_H
#define SDL_RPG_GAME_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "tiled_map.h"

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  bool running;
  Map map;
  SDL_Texture *atlas;
} Game;

#endif /* SDL_RPG_GAME_H */
