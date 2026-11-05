
#include "tiled_map.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <cJSON.h>
#include <stdlib.h>
#include <string.h>


static bool map_parse(const cJSON *root, Map *m, const char *path)
{
  (void)m; (void)path;
  return cJSON_IsObject(root);
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
