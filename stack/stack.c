#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "definitions.h"

Stack *new_stack(size_t capacity)
{
  Stack *stack = (Stack *)malloc(sizeof(Stack));
  if (stack == NULL)
  {
    return NULL;
  }

  stack->capacity = capacity;
  stack->length = 0;
  stack->data = calloc(sizeof(void *), capacity);
  if (stack->data == NULL)
  {
    free(stack);
    return NULL;
  }

  return stack;
}
