#include <stdlib.h>
#include <stdio.h>
#include "object.h"

int main(int argc, char *argv[])
{
  printf("%d\n", (new_integer(1))->data.INTEGER);
  printf("%f\n", (new_float(2.0))->data.FLOAT);

  printf("%f\n", (new_integer(1))->data.FLOAT);
  printf("%d\n", (new_float(1.0))->data.INTEGER);

  return 0;
}
