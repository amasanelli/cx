#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "object.h"
#include "definitions.h"

Object *new_object(void)
{
  return (Object *)calloc(1, sizeof(Object));
}

Object *new_integer(int value)
{
  Object *object = NULL;

  object = new_object();
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

  object = new_object();
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

  if (value == NULL)
  {
    return NULL;
  }

  object = new_object();
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

  object = new_object();
  if (object == NULL)
  {
    return NULL;
  }

  /*
  capacity 0 keeps elements NULL; first append grows via realloc(NULL, ...)
  */
  if (capacity > 0)
  {
    elements = (Object **)calloc(capacity, sizeof(Object *));
    if (elements == NULL)
    {
      free(object);
      return NULL;
    }
  }

  /*
  not needed: every field is assigned below, zeroing first is dead work
  memset(&array, 0, sizeof(Array));
  */

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
  size_t capacity;

  if (object == NULL || object->type != ARRAY || value == NULL)
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
  size_t i;
  size_t length;

  if (object == NULL || object->type != ARRAY || value == NULL)
  {
    return FALSE;
  }

  /*
  index directly: object_length/array_get would repeat the checks above
  */
  length = object->data.as_array.length;
  for (i = 0; i < length; i++)
  {
    if (object->data.as_array.elements[i] == value)
    {
      return TRUE;
    }
  }

  return FALSE;
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

/*
frees the object and its owned buffers; array elements belong to the caller
*/
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
    /*
    types already checked: strlen directly, skip object_length dispatch
    */
    len_a = strlen(a->data.as_string);
    len_b = strlen(b->data.as_string);
    temporary = (char *)calloc(len_a + len_b + 1, sizeof(char));
    if (temporary == NULL)
    {
      return NULL;
    }
    memcpy(temporary, a->data.as_string, len_a);
    memcpy(temporary + len_a, b->data.as_string, len_b);
    /*
    hand the buffer to the object directly: new_string would alloc and copy again
    */
    auxiliary = new_object();
    if (auxiliary == NULL)
    {
      free(temporary);
      return NULL;
    }
    auxiliary->type = STRING;
    auxiliary->data.as_string = temporary;
    return auxiliary;
  case ARRAY:
    if (b->type != ARRAY)
    {
      return NULL;
    }
    len_a = a->data.as_array.length;
    len_b = b->data.as_array.length;
    auxiliary = new_array(len_a + len_b);
    if (auxiliary == NULL)
    {
      return NULL;
    }
    /*
    capacity preallocated: copy directly, array_append cannot fail or grow
    */
    if (len_a > 0)
    {
      memcpy(auxiliary->data.as_array.elements, a->data.as_array.elements, len_a * sizeof(Object *));
    }
    if (len_b > 0)
    {
      memcpy(auxiliary->data.as_array.elements + len_a, b->data.as_array.elements, len_b * sizeof(Object *));
    }
    auxiliary->data.as_array.length = len_a + len_b;
    return auxiliary;
  default:
    return NULL;
  }
}
