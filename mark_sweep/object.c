#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "object.h"
#include "definitions.h"

int array_append(Object *object, Object *value)
{
  Object **temporary = NULL;
  size_t capacity;

  if (object == NULL || object->type != ARRAY)
  {
    return RET_ERR;
  }

  if (object->data.as_array.length == object->data.as_array.capacity)
  {
    capacity = object->data.as_array.capacity < MIN_CAPACITY ? MIN_CAPACITY : object->data.as_array.capacity * 2;

    temporary = (Object **)realloc(object->data.as_array.elements, sizeof(Object *) * capacity);
    if (temporary == NULL)
    {
      return RET_ERR;
    }

    object->data.as_array.capacity = capacity;
    object->data.as_array.elements = temporary;
  }

  object->data.as_array.elements[object->data.as_array.length++] = value;

  return RET_OK;
}

int array_contains(Object *object, Object *value)
{
  int i;

  if (object == NULL || object->type != ARRAY || value == NULL)
  {
    return 0;
  }

  for (i = 0; i < object_length(object); i++)
  {
    if (array_get(object, i) == value)
    {
      return 1;
    }
  }

  return 0;
}

int array_set(Object *object, Object *value, size_t index)
{
  if (object == NULL || object->type != ARRAY || index >= object->data.as_array.length || value == NULL)
  {
    return RET_ERR;
  }

  object->data.as_array.elements[index] = value;

  return RET_OK;
}

Object *array_get(Object *object, size_t index)
{
  if (object == NULL || object->type != ARRAY || index >= object->data.as_array.length)
  {
    return NULL;
  }

  return object->data.as_array.elements[index];
}

size_t object_length(Object *object)
{
  if (object == NULL)
  {
    return 0;
  }

  switch (object->type)
  {
  case INTEGER:
  case FLOAT:
    return 1;
  case STRING:
    return strlen(object->data.as_string);
  case ARRAY:
    return object->data.as_array.length;
  default:
    return 0;
  }
}

void object_free(Object *object)
{
  if (object == NULL)
  {
    return;
  }

  switch (object->type)
  {
  case STRING:
    free(object->data.as_string);
    break;
  case ARRAY:
    free(object->data.as_array.elements);
    break;
  default:
    break;
  }

  free(object);

  return;
}
