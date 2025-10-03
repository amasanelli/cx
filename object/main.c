#include <stdlib.h>
#include <stdio.h>
#include "object.h"

int main(int argc, char *argv[])
{
  Object *array = NULL;
  size_t i;

  printf("%d\n", (new_integer(1))->data.as_int);
  printf("%f\n", (new_float(2.0))->data.as_float);
  printf("%s\n", (new_string("hello"))->data.as_string);

  array = new_array(1);
  array_append(array, new_integer(2));
  printf("%d\n", (array_get(array, 0))->data.as_int);
  array_set(array, new_integer(1), 0);
  printf("%d\n", (array_get(array, 0))->data.as_int);
  array_append(array, new_integer(2));
  array_append(array, new_integer(3));
  array_append(array, new_integer(4));
  array_append(array, new_integer(5));

  for (i = 0; i < object_length(array); i++)
  {
    printf("%d\n", (array_get(array, i))->data.as_int);
  }

  return 0;
}
