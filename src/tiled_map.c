
#include "tiled_map.h"
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <cJSON.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL_error.h>


static bool map_number(const cJSON *o, const char *key, bool required, double fallback, double *out)
{
  const cJSON *val = cJSON_GetObjectItemCaseSensitive(o, key);
  if (!val) {
    if (required) return false;

    *out = fallback;
    return true;
  }

  if (!cJSON_IsNumber(val) || !isfinite(val->valuedouble))
  {
    return false;
  }

  *out = val->valuedouble;
  return true;
}

static bool map_int(const cJSON *o, const char *key, bool required, int fallback, int *out)
{
  double n;
  if (!map_number(o, key, required, fallback, &n) || floor(n) != n)
    return false;

  *out = (int)n;
  return true;
}

static bool map_bool(const cJSON *o, const char *key, bool fallback, bool *out)
{
  const cJSON *val = cJSON_GetObjectItemCaseSensitive(o, key);
  if (!val) {
    *out = fallback;
    return true;
  }

  if (!cJSON_IsBool(val)) return false;

  *out = cJSON_IsTrue(val);
  return true;
}

static bool map_string_eq(const cJSON *o, const char *key, const char *expected, bool required)
{
  const cJSON *val = cJSON_GetObjectItemCaseSensitive(o, key);

  if (!val && !required) return true;

  return cJSON_IsString(val) && !strcmp(val->valuestring, expected);
}

static bool map_tilesets(Map *m, const cJSON *o, const char *path)
{
  const cJSON *tilesets = cJSON_GetObjectItemCaseSensitive(o, "tilesets");
  if (!cJSON_IsArray(tilesets) || cJSON_GetArraySize(tilesets) != 1) {
    SDL_SetError("Error in tilesets, array length mismatch!");
    return false;
  }

  const cJSON *s = cJSON_GetArrayItem(tilesets, 0);
  if (!cJSON_IsObject(s)) return false;

  int tw, th = 0;
  if (
    !map_int(s, "tilewidth", true, 0, &tw) || tw != m->tile_width ||
    !map_int(s, "tileheight", true, 0, &th) || th != m->tile_height ||
    !map_int(s, "tilecount", true, 0, &m->tile_count) ||
    !map_int(s, "firstgid", true, 0, &m->firstgid) ||
    !map_int(s, "columns", true, 0, &m->columns) ||
    !map_int(s, "imagewidth", true, 0, &m->image_width) ||
    !map_int(s, "imageheight", true, 0, &m->image_height)
  ) {
    SDL_SetError("Error tilesets validating fields in set!");
    return false;
  }

  if (m->columns * tw != m->image_width) {
    SDL_SetError("Error in tilesets wrong image_width!");
    return false;
  }

  int rows = (m->tile_count + m->columns-1) / m->columns;
  if ((rows * th) > m->image_height) {
    SDL_SetError("Error in tilesets image_height is wrong!");
    return false;
  }

  const cJSON *image = cJSON_GetObjectItemCaseSensitive(s, "image");
  if (!cJSON_IsString(image)) {
    SDL_SetError("Error in tilesets image key not found!");
    return false;
  }

  // expected: maps/../assets/world.png
  // path => maps/start.json
  // image -> ../assets/world_tileset.png
  const char *slash = strrchr(path, '/');
  int prefix = slash ? (int)(slash-path+1) : 0;

  int n = image->valuestring[0] == '/'
    ? snprintf(m->image, sizeof m->image, "%s", image->valuestring)
    : snprintf(m->image, sizeof m->image, "%.*s%s", prefix, path, image->valuestring);

  return n >= 0 && (size_t)n < sizeof m->image;
}

static bool map_parse(const cJSON *root, Map *m, const char *path)
{
  if (!cJSON_IsObject(root)) return false;

  bool infinite;
  if (
    !map_string_eq(root, "type", "map", false) ||
    !map_string_eq(root, "orientation", "orthogonal", true) ||
    !map_string_eq(root, "renderorder", "right-down", false) ||
    !map_bool(root, "infinite", false, &infinite) ||
    !map_int(root, "width", true, 0, &m->width) ||
    !map_int(root, "height", true, 0, &m->height) ||
    !map_int(root, "tilewidth", true, 0, &m->tile_width) ||
    !map_int(root, "tileheight", true, 0, &m->tile_height) ||
    !map_tilesets(m, root, path)
  ) return false;

  return true;
}

bool map_read(Map *out, const char *path)
{
  if (!out || !path) return false;

  FILE *f = fopen(path, "rb");
  if (!f) {
    return false;
  }

  enum {
    LIMIT = 1024 * 1024
  };

  char *bytes = malloc(LIMIT+1);
  if (!bytes) {
    fclose(f);
    return false;
  }

  size_t n = fread(bytes, 1, LIMIT+1, f);
  bool ok = !ferror(f) && n <= LIMIT;

  if (fclose(f) != 0){
    ok = false;
  }
  if (!ok || memchr(bytes, 0, n))
  {
    free(bytes);
    return false;
  }

  bytes[n] = 0;
  cJSON *root = cJSON_ParseWithLengthOpts(
    bytes, n+1, NULL, true);

  Map *next = calloc(1, sizeof *next);
  ok = root && next && map_parse(root, next, path);

  if (ok) *out = *next;

  free(next);
  cJSON_Delete(root);
  free(bytes);

  return true;
}
