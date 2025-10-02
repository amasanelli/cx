#include "stack.h"

int main(int argc, char *argv[])
{
  Stack *stack;
  int test = 1;

  stack = new_stack(8);

  stack_push(stack, &test);

  return 0;
}
