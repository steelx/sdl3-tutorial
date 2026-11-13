
#include "tiled_map.h"
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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

static bool map_gid(const cJSON *v, int count, int *out)
{
  /* Deliberately reject flip flags until the renderer implements them. */
  if (!cJSON_IsNumber(v) || !isfinite(v->valuedouble) ||
        floor(v->valuedouble) != v->valuedouble || v->valuedouble < 0 || v->valuedouble > count) {
    return false;
  }
  *out = (int)v->valuedouble;
  return true;
}

static bool map_layer(Map *m, const cJSON *layer, bool *ground_seen, bool *collision_seen)
{
  const cJSON *name = cJSON_GetObjectItemCaseSensitive(layer, "name");
  if (!cJSON_IsString(name)) return false;

  bool bIsGroundLayer = strcmp(name->valuestring, "ground") == 0;
  bool bIsCollisionLayer = strcmp(name->valuestring, "collision") == 0;

  bool visible; double opacity = 0;
  if (
    (*ground_seen && bIsGroundLayer) || (*collision_seen && bIsCollisionLayer) ||
    !map_bool(layer, "visible", 0, &visible) ||
    !map_number(layer, "opacity", false, 1, &opacity)
  ) {
    SDL_SetError("map_layer layer has missing properties!");
    return false;
  }

  if (bIsGroundLayer) *ground_seen = true;
  if (bIsCollisionLayer) *collision_seen = true;


  // layer validations
  int id, w, h = 0;
  if (
    !map_string_eq(layer, "type", "tilelayer", false) ||
    !map_int(layer, "width", true, 0, &w) ||
    !map_int(layer, "height", true, 0, &h) ||
    !map_int(layer, "id", true, -1, &id)
  ) {
    SDL_SetError("map_layer validation failed for: %s\n", name->valuestring);
    return false;
  }

  MapLayer *next_layer = NULL;
  if (!bIsGroundLayer && !bIsCollisionLayer) {
    next_layer = &m->layers[m->layer_count++];
    snprintf(next_layer->name, sizeof next_layer->name, "%s", name->valuestring);
    next_layer->id = id;
    next_layer->opacity = (float)opacity;
    next_layer->visible = visible;
    next_layer->cells = calloc((size_t)w*h, sizeof *next_layer->cells);
  }

  const cJSON *data = cJSON_GetObjectItemCaseSensitive(layer, "data");
  if (!cJSON_IsArray(data) || cJSON_GetArraySize(data) != (w*h)) {
    SDL_SetError("Layer %s data mismatch!", name->valuestring);
    return false;
  }


  int i = 0; const cJSON *v;
  cJSON_ArrayForEach(v, data) {
    int gid;
    if (!map_gid(v, m->tile_count, &gid)) return false;

    //int x = i%w, y = i/w; but we will need for retrival
    if (bIsGroundLayer) m->ground[i] = gid;
    if (bIsCollisionLayer) m->collsions[i] = gid != 0;
    if (next_layer) next_layer->cells[i] = gid;
    ++i;
  }

  return true;
}

static bool map_parse(const cJSON *root, Map *m, const char *path)
{
  if (!cJSON_IsObject(root)) return false;

  bool infinite, ground, collision = false;
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

  size_t count = (size_t)m->width * m->height;
  m->ground = calloc(count, sizeof *m->ground);
  m->collsions = calloc(count, sizeof *m->collsions);
  if (!m->ground && !m->collsions) return false;

  const cJSON *layers = cJSON_GetObjectItemCaseSensitive(root, "layers");
  if (!cJSON_IsArray(layers)) {
    SDL_SetError("map_parse layers array not found!");
    return false;
  }

  const cJSON *layer;
  uint8_t visual_count = 0;
  cJSON_ArrayForEach(layer, layers) {
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(layer, "name");
    if (
      cJSON_IsString(name) &&
      strcmp("ground", name->valuestring) != 0 &&
      strcmp("collision", name->valuestring) != 0
    ) {
      ++visual_count;
    }
  }

  m->layer_count = 0;

  if (visual_count > 0) {
    m->layers = calloc(visual_count, sizeof *m->layers);
    if (!m->layers) return false;
  }
  else {
    m->layers = NULL;
  }

  cJSON_ArrayForEach(layer, layers) {
    if (!map_layer(m, layer, &ground, &collision)) return false;
  }

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

int get_map_gid(const Map *m, const int *cells, int x, int y)
{
  if (!m || !cells || x < 0 || y < 0 || x >= m->width || y >= m->height)
    return 0;

  return cells[(size_t)y*m->width + x];
}

bool get_collision_gid(const Map *m, int x, int y)
{
  if (!m || !m->collsions || x < 0 || y < 0 || x>=m->width || y >= m->height) return false;
  return m->collsions[(size_t)y*m->width + x];
}

void map_free(Map *m)
{
  if (!m) return;

  free(m->ground);
}
