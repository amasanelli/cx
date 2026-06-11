#include <stdlib.h>
#include <string.h>
#include "set.h"
#include "definitions.h"

void set_init(Set *set)
{
  memset(set, 0, sizeof(Set));
}

int set_contains(Set *set, void *item)
{
  size_t i;

  for (i = 0; i < set->length; i++)
  {
    if (set->items[i] == item)
      return 1;
  }

  return 0;
}

int set_add(Set *set, void *item)
{
  void **temporary = NULL;
  size_t capacity;

  if (set_contains(set, item))
  {
    return 0;
  }

  if (set->length == set->capacity)
  {
    capacity = set->capacity < MIN_CAPACITY ? MIN_CAPACITY : set->capacity * 2;

    temporary = (void **)realloc(set->items, sizeof(void *) * capacity);
    if (temporary == NULL)
    {
      return 0;
    }

    set->capacity = capacity;
    set->items = temporary;
  }

  set->items[set->length++] = item;

  return 1;
}

void set_free(Set *set)
{
  free(set->items);
  memset(set, 0, sizeof(Set));
}
