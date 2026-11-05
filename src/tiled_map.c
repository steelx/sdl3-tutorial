
#include "tiled_map.h"
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <cJSON.h>
#include <stdlib.h>
#include <string.h>

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

static bool map_parse(const cJSON *root, Map *m, const char *path)
{
  if (!cJSON_IsObject(root)) return false;

  (void)path;
  bool infinite;

  if (
    !map_string_eq(root, "type", "map", false) ||
    !map_string_eq(root, "orientation", "orthogonal", true) ||
    !map_string_eq(root, "renderorder", "right-down", false) ||
    !map_bool(root, "infinite", false, &infinite) ||
    !map_int(root, "width", true, 0, &m->width) ||
    !map_int(root, "height", true, 0, &m->height) ||
    !map_int(root, "tilewidth", true, 0, &m->tile_width) ||
    !map_int(root, "tileheight", true, 0, &m->tile_height)
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
