#include <stdio.h>
#include "stack.h"
#include "definitions.h"

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT(message, test)                                 \
  do                                                          \
  {                                                           \
    tests_run++;                                              \
    if (!(test))                                              \
    {                                                         \
      tests_failed++;                                         \
      printf("FAIL %s:%d %s\n", __FILE__, __LINE__, message); \
    }                                                         \
  } while (0)

static void test_new_stack(void)
{
  Stack *stack;

  stack = new_stack(8);
  ASSERT("new_stack returns a stack", stack != NULL);
  ASSERT("new stack is empty", stack->length == 0);
  ASSERT("new stack keeps requested capacity", stack->capacity == 8);
  ASSERT("new stack allocates data", stack->data != NULL);
  stack_free(stack);

  /*
  capacity 0 keeps data NULL; first push grows via realloc(NULL, ...)
  */
  stack = new_stack(0);
  ASSERT("new_stack(0) returns a stack", stack != NULL);
  ASSERT("new_stack(0) is empty", stack->length == 0);
  ASSERT("new_stack(0) has capacity 0", stack->capacity == 0);
  ASSERT("new_stack(0) keeps data NULL", stack->data == NULL);
  stack_free(stack);
}

static void test_push_pop_lifo(void)
{
  Stack *stack;
  int a = 1;
  int b = 2;
  int c = 3;

  stack = new_stack(8);

  ASSERT("push a", stack_push(stack, &a) == RET_OK);
  ASSERT("push b", stack_push(stack, &b) == RET_OK);
  ASSERT("push c", stack_push(stack, &c) == RET_OK);
  ASSERT("length tracks pushes", stack->length == 3);

  ASSERT("pop returns last pushed", stack_pop(stack) == &c);
  ASSERT("pop returns in reverse order", stack_pop(stack) == &b);
  ASSERT("pop returns first pushed last", stack_pop(stack) == &a);
  ASSERT("length tracks pops", stack->length == 0);

  stack_free(stack);
}

static void test_peek(void)
{
  Stack *stack;
  int a = 1;
  int b = 2;

  stack = new_stack(8);
  stack_push(stack, &a);
  stack_push(stack, &b);

  ASSERT("peek returns top", stack_peek(stack) == &b);
  ASSERT("peek does not remove", stack->length == 2);
  ASSERT("repeated peek returns same element", stack_peek(stack) == &b);

  stack_free(stack);
}

static void test_growth(void)
{
  Stack *stack;
  int values[10];
  size_t i;

  for (i = 0; i < 10; i++)
  {
    values[i] = (int)i;
  }

  /*
  small capacity grows to MIN_CAPACITY first, then doubles
  */
  stack = new_stack(2);
  for (i = 0; i < 3; i++)
  {
    ASSERT("push during growth succeeds", stack_push(stack, &values[i]) == RET_OK);
  }
  ASSERT("capacity grows to MIN_CAPACITY", stack->capacity == MIN_CAPACITY);

  for (i = 3; i < 10; i++)
  {
    ASSERT("push past MIN_CAPACITY succeeds", stack_push(stack, &values[i]) == RET_OK);
  }
  ASSERT("capacity doubles past MIN_CAPACITY", stack->capacity == MIN_CAPACITY * 2);
  ASSERT("length tracks all pushes", stack->length == 10);

  for (i = 10; i > 0; i--)
  {
    ASSERT("elements intact after realloc", stack_pop(stack) == &values[i - 1]);
  }

  stack_free(stack);

  /*
  growth from capacity 0 (data starts NULL)
  */
  stack = new_stack(0);
  ASSERT("push onto zero-capacity stack succeeds", stack_push(stack, &values[0]) == RET_OK);
  ASSERT("zero-capacity stack grows to MIN_CAPACITY", stack->capacity == MIN_CAPACITY);
  ASSERT("element readable after growth from 0", stack_pop(stack) == &values[0]);
  stack_free(stack);
}

static void test_empty(void)
{
  Stack *stack;

  stack = new_stack(8);
  ASSERT("pop on empty stack returns NULL", stack_pop(stack) == NULL);
  ASSERT("peek on empty stack returns NULL", stack_peek(stack) == NULL);
  ASSERT("empty pops do not corrupt length", stack->length == 0);
  stack_free(stack);
}

static void test_null_safety(void)
{
  Stack *stack;
  int a = 1;

  ASSERT("push on NULL stack errors", stack_push(NULL, &a) == RET_ERR);
  ASSERT("pop on NULL stack returns NULL", stack_pop(NULL) == NULL);
  ASSERT("peek on NULL stack returns NULL", stack_peek(NULL) == NULL);
  stack_free(NULL); /* must not crash */

  /*
  NULL elements are accepted: the stack stores any pointer value
  */
  stack = new_stack(8);
  ASSERT("push NULL element succeeds", stack_push(stack, NULL) == RET_OK);
  ASSERT("NULL element counts toward length", stack->length == 1);
  ASSERT("pop returns the stored NULL", stack_pop(stack) == NULL);
  ASSERT("NULL element pop updates length", stack->length == 0);
  stack_free(stack);
}

int main(void)
{
  test_new_stack();
  test_push_pop_lifo();
  test_peek();
  test_growth();
  test_empty();
  test_null_safety();

  printf("%d tests, %d failed\n", tests_run, tests_failed);

  return tests_failed > 0 ? 1 : 0;
}
