#include <stdlib.h>
#include <string.h>
#include "object.h"

Object *new_integer(int value)
{
  Object *object = NULL;

  object = (Object *)calloc(1, sizeof(Object));
  if (object == NULL)
  {
    return NULL;
  }

  object->type = INTEGER;
  object->data.as_int = value;

  return object;
}

Object *new_float(float value)
{
  Object *object = NULL;

  object = (Object *)calloc(1, sizeof(Object));
  if (object == NULL)
  {
    return NULL;
  }

  object->type = FLOAT;
  object->data.as_float = value;

  return object;
}

Object *new_string(char *value)
{
  Object *object = NULL;

  object = (Object *)calloc(1, sizeof(Object));
  if (object == NULL)
  {
    return NULL;
  }

  object->type = STRING;
  object->data.as_string = value;

  return object;
}

Object *new_array(size_t capacity)
{
  Object *object = NULL;
  Object **elements = NULL;
  Array array;

  if (capacity == 0)
  {
    return NULL;
  }

  memset(&array, 0, sizeof(Array));

  object = (Object *)calloc(1, sizeof(Object));
  if (object == NULL)
  {
    return NULL;
  }

  elements = (Object **)calloc(capacity, sizeof(Object *));
  if (elements == NULL)
  {
    free(object);
    return NULL;
  }
  array.elements = elements;
  array.length = 0;
  array.capacity = capacity;

  object->type = ARRAY;
  object->data.as_array = array;

  return object;
}
