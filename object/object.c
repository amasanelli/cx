#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "definitions.h"

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

int array_append(Object *object, Object *value)
{
  Object **temporary = NULL;

  if (object == NULL || object->type != ARRAY)
  {
    return RET_ERR;
  }

  if (object->data.as_array.length == object->data.as_array.capacity)
  {
    object->data.as_array.capacity *= 2;

    temporary = (Object **)realloc(object->data.as_array.elements, sizeof(Object *) * object->data.as_array.capacity);
    if (temporary == NULL)
    {
      object->data.as_array.capacity /= 2;
      return RET_ERR;
    }

    object->data.as_array.elements = temporary;
  }

  object->data.as_array.elements[object->data.as_array.length++] = value;

  return RET_OK;
}

int array_set(Object *object, Object *value, size_t index)
{
  if (object == NULL || object->type != ARRAY || index >= object->data.as_array.length)
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
