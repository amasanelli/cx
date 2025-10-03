#include <stdlib.h>
#include <stdio.h>
#include "object.h"

int main(int argc, char *argv[])
{
  Object *a = NULL;
  Object *b = NULL;

  /*
  Object *c = NULL;
  Object *d = NULL;

  a = new_integer(1);
  printf("%d\n", a->ref_count);

  b = new_array(1);
  array_append(b, a);
  printf("%d\n", a->ref_count);

  ref_count_dec(b);
  printf("%d\n", a->ref_count);
  b = NULL;

  b = new_array(1);
  array_append(b, a);
  printf("%d\n", a->ref_count);

  ref_count_dec(a);
  printf("%d\n", a->ref_count);
  a = NULL;

  printf("%d\n", b->data.as_array.elements[0]->ref_count);

  c = new_integer(1);
  printf("%d\n", c->ref_count);

  d = new_array(1);
  array_append(d, c);
  printf("%d\n", c->ref_count);

  array_append(b, c);
  printf("%d\n", c->ref_count);

  ref_count_dec(b);
  printf("%d\n", c->ref_count);
  b = NULL;

  ref_count_dec(d);
  printf("%d\n", c->ref_count);
  d = NULL;
  */

  a = new_array(2);
  printf("%d\n", a->ref_count);

  b = new_array(2);
  printf("\n");
  printf("%d\n", b->ref_count);

  array_append(a, b);
  printf("\n");
  printf("%d\n", a->ref_count);
  printf("%d\n", b->ref_count);

  array_append(b, a);
  printf("\n");
  printf("%d\n", a->ref_count);
  printf("%d\n", b->ref_count);

  ref_count_dec(b);
  printf("\n");
  printf("%d\n", a->ref_count);
  printf("%d\n", b->ref_count);

  ref_count_dec(a);
  printf("\n");
  printf("%d\n", a->ref_count);
  printf("%d\n", b->ref_count);

  return 0;
}
