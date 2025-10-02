#include <stdlib.h>
#include <stdio.h>
#include "object.h"

int main(int argc, char *argv[])
{
  Object *object = NULL;

  object = new_integer(1);

  printf("%d\n", object->data.integer);

  return 0;
}
