#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "object.h"
#include "definitions.h"
#include "set.h"

void ref_count_inc(Object *object)
{
  if (object == NULL)
  {
    return;
  }

  object->ref_count++;

  return;
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

void ref_count_dec_safe(Object *object, Set *visited)
{
  size_t i;
  size_t length;

  if (object == NULL || visited == NULL)
  {
    return;
  }

  /*
  visited holds only objects whose teardown is in progress higher up this
  recursion (a cycle pointing back into a dying object); their pending free
  already accounts for everything, so skip. it must NOT dedupe decrements:
  an array holding the same child twice owns two references and must give
  back two on teardown.
  */
  if (set_contains(visited, object))
  {
    return;
  }

  object->ref_count--;

  /*
  children lose a reference only when this object is actually freed:
  a surviving array still points at them
  refcounting therefore cannot collect cycles - that is the gap mark_sweep exists to fill
  */
  if (object->ref_count > 0)
  {
    return;
  }

  /*
  negative count means a caller bug (dec of a dead object): do not double free
  */
  if (object->ref_count < 0)
  {
    return;
  }

  if (object->type == ARRAY)
  {
    /*
    mark teardown in progress BEFORE recursing so cycles back into this
    object cannot re-enter it. on alloc failure the children leak but the
    recursion stays safe.
    */
    if (set_add(visited, object))
    {
      /*
      index directly: object_length/array_get would repeat the checks above
      */
      length = object->data.as_array.length;
      for (i = 0; i < length; i++)
      {
        ref_count_dec_safe(object->data.as_array.elements[i], visited);
      }
    }
  }

  object_free(object);

  return;
}

void ref_count_dec(Object *object)
{
  Set visited;

  set_init(&visited);
  ref_count_dec_safe(object, &visited);
  set_free(&visited);
}

Object *new_object(void)
{
  Object *object = NULL;

  object = (Object *)calloc(1, sizeof(Object));
  if (object == NULL)
  {
    return NULL;
  }

  object->ref_count = 1;

  return object;
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

  ref_count_inc(value);

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

  /*
  inc before dec: if value IS the old element with ref_count 1, the
  reverse order frees it and then touches freed memory
  */
  ref_count_inc(value);
  ref_count_dec(object->data.as_array.elements[index]);

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
  size_t i;
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
    capacity preallocated so this cannot fail or grow; copy and take the
    reference array_append would have taken in one pass per source
    */
    for (i = 0; i < len_a; i++)
    {
      auxiliary->data.as_array.elements[i] = a->data.as_array.elements[i];
      ref_count_inc(a->data.as_array.elements[i]);
    }
    for (i = 0; i < len_b; i++)
    {
      auxiliary->data.as_array.elements[len_a + i] = b->data.as_array.elements[i];
      ref_count_inc(b->data.as_array.elements[i]);
    }
    auxiliary->data.as_array.length = len_a + len_b;
    return auxiliary;
  default:
    return NULL;
  }
}
