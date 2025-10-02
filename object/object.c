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
  object->data.INTEGER = value;

  return object;
}

Object *new_float(float value)
{
  Object *object = (Object *)malloc(sizeof(Object));
  if (object == NULL)
  {
    return NULL;
  }

  object->type = FLOAT;
  object->data.FLOAT = value;

  return object;
}
