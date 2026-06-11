#include <stdio.h>
#include <string.h>
#include "vm.h"
#include "object.h"
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

static void test_stack(void)
{
  Stack *stack;
  int a = 1;
  int b = 2;

  stack = new_stack(2);
  ASSERT("new stack is empty", stack->length == 0);

  stack_push(stack, &a);
  stack_push(stack, &b);
  ASSERT("peek returns top without removing", stack_peek(stack) == &b && stack->length == 2);
  stack_push(stack, &a);
  ASSERT("capacity grows to MIN_CAPACITY", stack->capacity == MIN_CAPACITY);
  ASSERT("pop returns in reverse order", stack_pop(stack) == &a && stack_pop(stack) == &b && stack_pop(stack) == &a);
  ASSERT("pop on empty stack returns NULL", stack_pop(stack) == NULL);
  ASSERT("peek on empty stack returns NULL", stack_peek(stack) == NULL);
  stack_free(stack);

  ASSERT("push on NULL stack errors", stack_push(NULL, &a) == RET_ERR);
  ASSERT("pop on NULL stack returns NULL", stack_pop(NULL) == NULL);
  stack_free(NULL); /* must not crash */
}

static void test_stack_remove_nulls(void)
{
  Stack *stack;
  int a = 1;
  int b = 2;
  size_t i;

  stack = new_stack(8);
  stack_push(stack, &a);
  stack_push(stack, NULL);
  stack_push(stack, &b);
  stack_push(stack, NULL);

  stack_remove_nulls(stack);
  ASSERT("nulls are removed", stack->length == 2);
  ASSERT("survivors keep their order", stack->data[0] == &a && stack->data[1] == &b);
  for (i = stack->length; i < stack->capacity; i++)
  {
    ASSERT("dead slots are cleared", stack->data[i] == NULL);
  }
  stack_free(stack);

  stack = new_stack(8);
  stack_push(stack, NULL);
  stack_push(stack, NULL);
  stack_remove_nulls(stack);
  ASSERT("all-NULL stack compacts to empty", stack->length == 0);
  stack_remove_nulls(stack); /* empty stack: must not crash */
  stack_free(stack);

  stack_remove_nulls(NULL); /* must not crash */
  ASSERT("stack_remove_nulls(NULL) is safe", 1);
}

static void test_object_model(void)
{
  Object *integer;
  Object *floating;
  Object *string;
  Object *array;
  Object *result;
  char source[6];

  strcpy(source, "hello");

  integer = new_integer(42);
  ASSERT("new_integer stores value", integer->type == INTEGER && integer->data.as_int == 42);
  ASSERT("new objects start unmarked", integer->marked == FALSE);

  floating = new_float(2.5f);
  ASSERT("new_float stores value", floating->type == FLOAT && floating->data.as_float == 2.5f);

  string = new_string(source);
  ASSERT("new_string copies content", strcmp(string->data.as_string, "hello") == 0);
  source[0] = 'X';
  ASSERT("new_string buffer is independent", strcmp(string->data.as_string, "hello") == 0);
  ASSERT("new_string(NULL) returns NULL", new_string(NULL) == NULL);

  array = new_array(1);
  ASSERT("new_array is empty", array->type == ARRAY && object_length(array) == 0);

  ASSERT("append succeeds", array_append(array, integer) == RET_OK);
  ASSERT("append past capacity grows", array_append(array, floating) == RET_OK);
  ASSERT("length tracks appends", object_length(array) == 2);
  ASSERT("get returns the stored pointer", array_get(array, 0) == integer);
  ASSERT("get past length returns NULL", array_get(array, 2) == NULL);
  ASSERT("set replaces element", array_set(array, floating, 0) == RET_OK && array_get(array, 0) == floating);
  ASSERT("set past length errors", array_set(array, integer, 2) == RET_ERR);
  ASSERT("append to non-array errors", array_append(integer, array) == RET_ERR);
  ASSERT("contains finds stored pointer", array_contains(array, floating) == TRUE);
  ASSERT("contains compares identity", array_contains(array, string) == FALSE);

  ASSERT("integer length is 1", object_length(integer) == 1);
  ASSERT("string length is strlen", object_length(string) == 5);
  ASSERT("NULL length is 0", object_length(NULL) == 0);

  result = object_add(integer, integer);
  ASSERT("int+int sums", result->type == INTEGER && result->data.as_int == 84);
  object_free(result);
  result = object_add(integer, floating);
  ASSERT("int+float is FLOAT", result->type == FLOAT && result->data.as_float == 44.5f);
  object_free(result);
  result = object_add(string, string);
  ASSERT("string+string concatenates", strcmp(result->data.as_string, "hellohello") == 0);
  object_free(result);
  result = object_add(array, array);
  ASSERT("array+array is shallow with summed length", object_length(result) == 4 && array_get(result, 0) == floating);
  object_free(result);
  ASSERT("mismatched add is NULL", object_add(integer, string) == NULL);
  ASSERT("NULL operand add is NULL", object_add(NULL, integer) == NULL);

  object_free(integer);
  object_free(floating);
  object_free(string);
  object_free(array);
  object_free(NULL); /* must not crash */
}

static void test_vm_lifecycle(void)
{
  VirtualMachine *vm;

  vm = new_vm();
  ASSERT("new_vm returns a vm", vm != NULL);
  ASSERT("new vm has no frames", vm->frames->length == 0);
  ASSERT("new vm tracks no objects", vm->objects->length == 0);

  vm_frame_free(vm); /* no frames: must not crash */
  vm_collect_garbage(vm); /* nothing to collect: must not crash */
  ASSERT("collect on empty vm is safe", vm->objects->length == 0);

  vm_free(vm);
  vm_free(NULL); /* must not crash */
  vm_collect_garbage(NULL); /* must not crash */
  vm_frame_free(NULL); /* must not crash */
  ASSERT("NULL vm calls are safe", 1);
}

static void test_frames(void)
{
  VirtualMachine *vm;
  Frame *frame;
  Object *object;

  vm = new_vm();
  frame = vm_new_frame(vm);
  ASSERT("vm_new_frame returns a frame", frame != NULL);
  ASSERT("frame is pushed onto the vm", vm->frames->length == 1 && stack_peek(vm->frames) == frame);
  ASSERT("frame points back at its vm", frame->vm == vm);
  ASSERT("frame starts with no references", frame->references->length == 0);
  ASSERT("vm_new_frame on NULL vm returns NULL", vm_new_frame(NULL) == NULL);

  object = new_integer(1);
  ASSERT("reference succeeds", frame_reference_object(frame, object) == RET_OK);
  ASSERT("referenced object is tracked by the vm", vm->objects->length == 1 && stack_peek(vm->objects) == object);
  ASSERT("frame holds the reference", frame->references->length == 1 && stack_peek(frame->references) == object);
  ASSERT("reference on NULL frame errors", frame_reference_object(NULL, object) == RET_ERR);
  ASSERT("reference of NULL object errors", frame_reference_object(frame, NULL) == RET_ERR);

  vm_frame_free(vm);
  ASSERT("vm_frame_free pops the frame", vm->frames->length == 0);
  ASSERT("the object stays tracked until the next sweep", vm->objects->length == 1);

  vm_free(vm); /* frees the orphaned object too */
}

static void test_shared_references_track_once(void)
{
  VirtualMachine *vm;
  Frame *frame1;
  Frame *frame2;
  Object *object;

  vm = new_vm();
  frame1 = vm_new_frame(vm);
  frame2 = vm_new_frame(vm);
  object = new_integer(1);

  frame_reference_object(frame1, object);
  frame_reference_object(frame2, object);
  frame_reference_object(frame2, object);
  /*
  duplicate tracking would double free the object at sweep or vm_free
  */
  ASSERT("shared object is tracked once", vm->objects->length == 1);
  ASSERT("first frame holds its reference", frame1->references->length == 1);
  ASSERT("repeat references still stack per frame", frame2->references->length == 2);

  vm_frame_free(vm);
  vm_collect_garbage(vm);
  ASSERT("object survives while one frame remains", vm->objects->length == 1);
  ASSERT("survivor data is intact", object->data.as_int == 1);

  vm_frame_free(vm);
  vm_collect_garbage(vm);
  ASSERT("object swept once after all frames die", vm->objects->length == 0);

  vm_free(vm);
}

static void test_gc_root_survival(void)
{
  VirtualMachine *vm;
  Frame *frame;
  Object *object;

  vm = new_vm();
  frame = vm_new_frame(vm);
  object = new_integer(1);
  frame_reference_object(frame, object);

  vm_collect_garbage(vm);
  ASSERT("object referenced by a live frame survives", vm->objects->length == 1);
  ASSERT("survivor is the same object", stack_peek(vm->objects) == object);
  ASSERT("sweep resets the mark", object->marked == FALSE);
  ASSERT("survivor data is intact", object->data.as_int == 1);

  vm_free(vm);
}

static void test_gc_unreachable(void)
{
  VirtualMachine *vm;
  Frame *frame;
  Object *object;

  vm = new_vm();
  frame = vm_new_frame(vm);
  object = new_integer(1);
  frame_reference_object(frame, object);

  vm_frame_free(vm);
  vm_collect_garbage(vm);
  ASSERT("object of a dead frame is swept", vm->objects->length == 0);

  vm_free(vm);
}

static void test_gc_trace_reachability(void)
{
  VirtualMachine *vm;
  Frame *frame1;
  Frame *frame2;
  Object *root;
  Object *child;

  vm = new_vm();
  frame1 = vm_new_frame(vm);
  root = new_array(1);
  frame_reference_object(frame1, root);

  /*
  the child's only root goes away with frame2; it must survive the
  collection because root still reaches it through the array
  */
  frame2 = vm_new_frame(vm);
  child = new_integer(7);
  frame_reference_object(frame2, child);
  array_append(root, child);

  vm_frame_free(vm);
  vm_collect_garbage(vm);
  ASSERT("child reachable through a root array survives", vm->objects->length == 2);
  ASSERT("root still holds the child", array_get(root, 0) == child);
  ASSERT("child data is intact", child->data.as_int == 7);
  ASSERT("trace marks are reset", child->marked == FALSE);

  vm_free(vm);
}

static void test_gc_collects_cycles(void)
{
  VirtualMachine *vm;
  Frame *frame1;
  Frame *frame2;
  Object *object1;
  Object *object2;

  /*
  the teaching point: the cycle that leaks under ref_count is collected
  here once no frame reaches it
  */
  vm = new_vm();
  frame1 = vm_new_frame(vm);
  object1 = new_array(1);
  frame_reference_object(frame1, object1);

  frame2 = vm_new_frame(vm);
  object2 = new_array(1);
  frame_reference_object(frame2, object2);

  array_append(object1, object2);
  array_append(object2, object1);

  vm_frame_free(vm);
  vm_collect_garbage(vm);
  ASSERT("cycle reachable from a live frame survives", vm->objects->length == 2);
  ASSERT("cycle links are intact", array_get(object1, 0) == object2 && array_get(object2, 0) == object1);

  vm_frame_free(vm);
  vm_collect_garbage(vm);
  ASSERT("unreachable cycle is fully collected", vm->objects->length == 0);
  ASSERT("no frames remain", vm->frames->length == 0);

  vm_free(vm);
}

int main(void)
{
  test_stack();
  test_stack_remove_nulls();
  test_object_model();
  test_vm_lifecycle();
  test_frames();
  test_shared_references_track_once();
  test_gc_root_survival();
  test_gc_unreachable();
  test_gc_trace_reachability();
  test_gc_collects_cycles();

  printf("%d tests, %d failed\n", tests_run, tests_failed);

  return tests_failed > 0 ? 1 : 0;
}
