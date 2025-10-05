#include <stdio.h>
#include "stack.h"

int main(int argc, char *argv[])
{
  Stack *stack;
  int test = 1;
  void *check;

  stack = new_stack(8);

  stack_push(stack, &test);
  check = stack_peek(stack);
  if (check != NULL)
  {
    printf("%d\n", *(int *)check); /* 1 */
  }
  check = stack_pop(stack);
  if (check != NULL)
  {
    printf("%d\n", *(int *)check); /* 1 */
  }
  check = stack_peek(stack);
  if (check != NULL)
  {
    printf("%d\n", *(int *)check);
  }
  stack_free(stack);
  stack = NULL;
  check = stack_pop(stack);
  if (check != NULL)
  {
    printf("%d\n", *(int *)check);
  }

  return 0;
}
