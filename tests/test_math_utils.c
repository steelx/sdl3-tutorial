#include "math_utils.h"

#include <stdio.h>

int main(void) {
  if (add(20, 22) != 42 || add(-5, 2) != -3 || add(0, 0) != 0) {
    fprintf(stderr, "add() returned an unexpected result\n");
    return 1;
  }
  puts("All math_utils tests passed.");
  return 0;
}
