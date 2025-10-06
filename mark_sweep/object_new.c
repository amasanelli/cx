#include <string.h>
#include "object_new.h"
#include "object.h"
#include "vm.h"

Object *new_object(VirtualMachine *vm)
{
  Object *object = NULL;

  object = (Object *)calloc(1, sizeof(Object));
  if (object == NULL)
  {
    return NULL;
  }

  vm_track_object(vm, object);

  return object;
}

Object *new_integer(VirtualMachine *vm, int value)
{
  Object *object = NULL;

  object = new_object(vm);
  if (object == NULL)
  {
    return NULL;
  }

  object->type = INTEGER;
  object->data.as_int = value;

  return object;
}

Object *new_float(VirtualMachine *vm, float value)
{
  Object *object = NULL;

  object = new_object(vm);
  if (object == NULL)
  {
    return NULL;
  }

  object->type = FLOAT;
  object->data.as_float = value;

  return object;
}

Object *new_string(VirtualMachine *vm, char *value)
{
  Object *object = NULL;
  size_t length;
  char *auxiliary;

  object = new_object(vm);
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

Object *new_array(VirtualMachine *vm, size_t capacity)
{
  Object *object = NULL;
  Object **elements = NULL;
  Array array;

  if (capacity == 0)
  {
    return NULL;
  }

  object = new_object(vm);
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

Object *object_add(VirtualMachine *vm, Object *a, Object *b)
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
      return new_integer(vm, a->data.as_int + b->data.as_int);
    case FLOAT:
      return new_float(vm, a->data.as_int + b->data.as_float);
    default:
      return NULL;
    }
  case FLOAT:
    switch (b->type)
    {
    case INTEGER:
      return new_float(vm, a->data.as_float + b->data.as_int);
    case FLOAT:
      return new_float(vm, a->data.as_float + b->data.as_float);
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
    auxiliary = new_string(vm, temporary);
    free(temporary);
    return auxiliary;
  case ARRAY:
    if (b->type != ARRAY)
    {
      return NULL;
    }
    len_a = object_length(a);
    len_b = object_length(b);
    auxiliary = new_array(vm, len_a + len_b);
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
