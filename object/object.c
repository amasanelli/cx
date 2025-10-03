#include <stdlib.h>
#include <stdio.h>
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
  size_t length;
  char *auxiliary;

  object = (Object *)calloc(1, sizeof(Object));
  if (object == NULL)
  {
    return NULL;
  }

  length = strlen(value);
  auxiliary = calloc(length + 1, sizeof(char));
  if (auxiliary == NULL)
  {
    free(object);
    return NULL;
  }
  memcpy(auxiliary, value, length);

  object->type = STRING;
  object->data.as_string = auxiliary;

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

  memset(&array, 0, sizeof(Array));

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

Object *object_add(Object *a, Object *b)
{
  char *temporary;
  Object *auxiliary;
  int i;
  size_t len_a;
  size_t len_b;

  if (a == NULL || b == NULL)
  {
    return NULL;
  }

  switch (a->type)
  {
  case INTEGER:
    switch (b->type)
    {
    case INTEGER:
      return new_integer(a->data.as_int + b->data.as_int);
    case FLOAT:
      return new_float(a->data.as_int + b->data.as_float);
    default:
      return NULL;
    }
  case FLOAT:
    switch (b->type)
    {
    case INTEGER:
      return new_float(a->data.as_float + b->data.as_int);
    case FLOAT:
      return new_float(a->data.as_float + b->data.as_float);
    default:
      return NULL;
    }
  case STRING:
    if (b->type != STRING)
    {
      return NULL;
    }
    len_a = object_length(a);
    len_b = object_length(b);
    temporary = (char *)calloc(len_a + len_b + 1, sizeof(char));
    if (temporary == NULL)
    {
      return NULL;
    }
    memcpy(temporary, a->data.as_string, len_a);
    memcpy(temporary + len_a, b->data.as_string, len_b);
    auxiliary = new_string(temporary);
    free(temporary);
    return auxiliary;
  case ARRAY:
    if (b->type != ARRAY)
    {
      return NULL;
    }
    len_a = object_length(a);
    len_b = object_length(b);
    auxiliary = new_array(len_a + len_b);
    if (auxiliary == NULL)
    {
      return NULL;
    }
    for (i = 0; i < len_a; i++)
    {
      array_append(auxiliary, array_get(a, i));
    }
    for (i = 0; i < len_b; i++)
    {
      array_append(auxiliary, array_get(b, i));
    }
    return auxiliary;
  default:
    return NULL;
  }
}
