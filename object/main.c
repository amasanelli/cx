#include <stdlib.h>
#include <stdio.h>
#include "object.h"

int main(int argc, char *argv[])
{
  Object *array = NULL;
  /* Object *a = NULL; */
  /* Object *b = NULL; */
  Object *x = NULL;
  size_t i;

  /* printf("%d\n", (new_integer(1))->data.as_int); */
  /* printf("%f\n", (new_float(2.0))->data.as_float); */
  /* printf("%s\n", (new_string("hello"))->data.as_string); */

  array = new_array(1);
  x = new_integer(2);
  array_append(array, x);
  /* printf("%d\n", (array_get(array, 0))->data.as_int); */
  /* array_set(array, new_integer(1), 0); */
  /* printf("%d\n", (array_get(array, 0))->data.as_int); */
  array_append(array, new_integer(2));
  array_append(array, new_integer(3));
  array_append(array, new_integer(4));
  array_append(array, new_integer(5));

  for (i = 0; i < object_length(array); i++)
  {
    printf("%d\n", (array_get(array, i))->data.as_int);
  }

  printf("%d\n", array_contains(array, x));

  /* printf("%d\n", (object_add(new_integer(1), new_integer(1)))->data.as_int); */
  /* printf("%f\n", (object_add(new_integer(1), new_float(1.0)))->data.as_float); */
  /* printf("%f\n", (object_add(new_float(1.0), new_integer(1)))->data.as_float); */
  /* printf("%f\n", (object_add(new_float(1.0), new_float(1.0)))->data.as_float); */
  /* printf("%s\n", (object_add(new_string("hello"), new_string(", world!")))->data.as_string); */

  /* a = new_array(1); */
  /* array_append(a, new_integer(2)); */
  /* printf("%d\n", (array_get(a, 0))->data.as_int); */
  /* b = new_array(1); */
  /* array_append(b, new_integer(1)); */
  /* printf("%d\n", (array_get(b, 0))->data.as_int); */
  /* array = object_add(a, b); */
  /* for (i = 0; i < object_length(array); i++) */
  /* { */
  /*   printf("%d\n", (array_get(array, i))->data.as_int); */
  /* } */

  return 0;
}
