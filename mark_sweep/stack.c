#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "definitions.h"

Stack *new_stack(size_t capacity)
{
  Stack *stack = NULL;

  stack = (Stack *)calloc(1, sizeof(Stack));
  if (stack == NULL)
  {
    return NULL;
  }

  stack->capacity = capacity;
  stack->length = 0;

  /*
  capacity 0 keeps data NULL; first push grows via realloc(NULL, ...)
  */
  if (capacity > 0)
  {
    stack->data = (void **)calloc(capacity, sizeof(void *));
    if (stack->data == NULL)
    {
      free(stack);
      return NULL;
    }
  }

  return stack;
}

int stack_push(Stack *stack, void *object)
{
  void **temporary = NULL;
  size_t capacity;

  if (stack == NULL)
  {
    return RET_ERR;
  }

  if (stack->length == stack->capacity)
  {
    capacity = stack->capacity < MIN_CAPACITY ? MIN_CAPACITY : stack->capacity * 2;

    temporary = (void **)realloc(stack->data, sizeof(void *) * capacity);
    if (temporary == NULL)
    {
      return RET_ERR;
    }

    stack->capacity = capacity;
    stack->data = temporary;
  }

  stack->data[stack->length++] = object;

  return RET_OK;
}

void *stack_pop(Stack *stack)
{
  if (stack == NULL || stack->length == 0)
  {
    return NULL;
  }

  return stack->data[--stack->length];
}

void stack_free(Stack *stack)
{
  if (stack == NULL)
  {
    return;
  }

  if (stack->data != NULL)
  {
    free(stack->data);
  }

  free(stack);

  return;
}

void *stack_peek(Stack *stack)
{
  if (stack == NULL || stack->length == 0)
  {
    return NULL;
  }

  return stack->data[stack->length - 1];
}

void stack_remove_nulls(Stack *stack)
{
  size_t i;
  void *object;
  size_t length = 0;

  if (stack == NULL || stack->length == 0)
  {
    return;
  }

  for (i = 0; i < stack->length; i++)
  {
    object = stack->data[i];
    if (object != NULL)
    {
      stack->data[length++] = object;
    }
  }

  stack->length = length;

  /*
  keep dead slots NULL so stale pointers never linger after a sweep
  */
  for (i = length; i < stack->capacity; i++)
  {
    stack->data[i] = NULL;
  }

  return;
}
