#ifndef _STACK_
#define _STACK_

#include <stdlib.h>

typedef struct
{
  size_t length;
  size_t capacity;
  void **data;
} Stack;

Stack *new_stack(size_t capacity);
int stack_push(Stack *stack, void *object);
void *stack_pop(Stack *stack);

#endif
