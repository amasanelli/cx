#include <stdlib.h>
#include <stdio.h>
#include "object.h"

int main(int argc, char *argv[])
{
  printf("%d\n", (new_integer(1))->data.as_int);
  printf("%f\n", (new_float(2.0))->data.as_float);
  printf("%s\n", (new_string("hello"))->data.as_string);

  return 0;
}
