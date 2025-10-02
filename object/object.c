#include <stdlib.h>
#include "object.h"

Object *new_integer(int value)
{
  Object *object = (Object *)malloc(sizeof(Object));
  if (object == NULL)
  {
    return NULL;
  }

  object->type = INTEGER;
  object->data.integer = value;

  return object;
}
