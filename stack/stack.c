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
  stack->data = (void **)calloc(capacity, sizeof(void *));
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
    stack->capacity *= 2;

    temporary = (void **)realloc(stack->data, sizeof(void *) * stack->capacity);
    if (temporary == NULL)
    {
      stack->capacity /= 2;
      return RET_ERR;
    }

    stack->data = temporary;
  }

  stack->data[stack->length++] = object;

  return RET_OK;
}

void *stack_pop(Stack *stack)
{
  if (stack->length == 0)
  {
    return NULL;
  }

  return stack->data[--stack->length];
}
