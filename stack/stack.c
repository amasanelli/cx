#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "definitions.h"

Stack *new_stack(size_t capacity)
{
  Stack *stack = NULL;

  stack = (Stack *)malloc(sizeof(Stack));
  if (stack == NULL)
  {
    return NULL;
  }

  stack->capacity = capacity;
  stack->length = 0;
  stack->data = (void **)calloc(sizeof(void *), capacity);
  if (stack->data == NULL)
  {
    free(stack);
    return NULL;
  }

  return stack;
}

int stack_push(Stack *stack, void *object)
{
  void **temporary = NULL;

  if (stack->length == stack->capacity)
  {
    stack->capacity += stack->capacity;

    temporary = (void **)realloc(stack->data, sizeof(void *) * stack->capacity);
    if (temporary == NULL)
    {
      stack->capacity -= stack->capacity;
      return RET_ERR;
    }

    stack->data = temporary;
  }

  stack->data[stack->length] = object;
  stack->length++;

  return RET_OK;
}
