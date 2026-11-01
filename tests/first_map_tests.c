#include <stdio.h>
#define CHECK(x)                                                                       \
  do                                                                                 \
{                                                                                  \
  if (!(x))                                                                      \
  {                                                                              \
    fprintf(stderr, "Failed line %d: %s\n", __LINE__, #x);                     \
    return 1;                                                                  \
  }                                                                              \
} while (0)


int main(void)
{
  return 0;
}

