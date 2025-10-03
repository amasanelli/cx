#ifndef _SET_
#define _SET_

#include <stdlib.h>

typedef struct
{
  void **items;
  size_t length;
  size_t capacity;
} Set;

void set_init(Set *set);
int set_contains(Set *set, void *item);
int set_add(Set *set, void *item);
void set_free(Set *set);

#endif
