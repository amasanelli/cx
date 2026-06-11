#include <stdio.h>
#include <string.h>
#include "object.h"
#include "set.h"
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

static void test_set(void)
{
  Set set;
  int items[10];
  size_t i;

  set_init(&set);
  ASSERT("set starts empty", set.length == 0);
  ASSERT("set starts without storage", set.items == NULL);

  ASSERT("add returns TRUE", set_add(&set, &items[0]) == TRUE);
  ASSERT("add stores the item", set_contains(&set, &items[0]) == TRUE);
  ASSERT("duplicate add returns FALSE", set_add(&set, &items[0]) == FALSE);
  ASSERT("duplicate add does not grow", set.length == 1);
  ASSERT("missing item is not contained", set_contains(&set, &items[1]) == FALSE);

  for (i = 1; i < 10; i++)
  {
    ASSERT("add past MIN_CAPACITY succeeds", set_add(&set, &items[i]) == TRUE);
  }
  ASSERT("length tracks all adds", set.length == 10);
  ASSERT("capacity doubles past MIN_CAPACITY", set.capacity == MIN_CAPACITY * 2);
  for (i = 0; i < 10; i++)
  {
    ASSERT("items intact after growth", set_contains(&set, &items[i]) == TRUE);
  }

  set_free(&set);
  ASSERT("set_free resets length", set.length == 0);
  ASSERT("set_free resets storage", set.items == NULL);
}

static void test_constructors(void)
{
  Object *integer;
  Object *floating;
  Object *string;
  Object *array;
  char source[6];

  strcpy(source, "hello");

  integer = new_integer(42);
  ASSERT("new_integer stores value", integer->type == INTEGER && integer->data.as_int == 42);
  ASSERT("new integer starts with one reference", integer->ref_count == 1);

  floating = new_float(2.5f);
  ASSERT("new_float stores value", floating->type == FLOAT && floating->data.as_float == 2.5f);
  ASSERT("new float starts with one reference", floating->ref_count == 1);

  string = new_string(source);
  ASSERT("new_string copies content", strcmp(string->data.as_string, "hello") == 0);
  source[0] = 'X';
  ASSERT("new_string buffer is independent", strcmp(string->data.as_string, "hello") == 0);
  ASSERT("new string starts with one reference", string->ref_count == 1);
  ASSERT("new_string(NULL) returns NULL", new_string(NULL) == NULL);

  array = new_array(2);
  ASSERT("new_array is empty", array->type == ARRAY && object_length(array) == 0);
  ASSERT("new array starts with one reference", array->ref_count == 1);

  ref_count_dec(integer);
  ref_count_dec(floating);
  ref_count_dec(string);
  ref_count_dec(array);
}

static void test_append_takes_reference(void)
{
  Object *array;
  Object *child;

  array = new_array(1);
  child = new_integer(1);

  ASSERT("append succeeds", array_append(array, child) == RET_OK);
  ASSERT("append increments the child", child->ref_count == 2);
  ASSERT("append leaves the container alone", array->ref_count == 1);

  ASSERT("append to NULL errors", array_append(NULL, child) == RET_ERR);
  ASSERT("append NULL value errors", array_append(array, NULL) == RET_ERR);
  ASSERT("append to non-array errors", array_append(child, array) == RET_ERR);

  ref_count_dec(array);
  /*
  teardown of the array gave back its reference: only ours remains
  */
  ASSERT("container teardown releases the child", child->ref_count == 1);
  ref_count_dec(child);
}

static void test_array_get_set(void)
{
  Object *array;
  Object *old;
  Object *new;

  array = new_array(2);
  old = new_integer(1);
  new = new_integer(2);
  array_append(array, old);

  ASSERT("get returns the stored pointer", array_get(array, 0) == old);
  ASSERT("get past length returns NULL", array_get(array, 1) == NULL);
  ASSERT("get on NULL returns NULL", array_get(NULL, 0) == NULL);
  ASSERT("get on non-array returns NULL", array_get(old, 0) == NULL);

  ASSERT("set replaces element", array_set(array, new, 0) == RET_OK);
  ASSERT("set stored the new pointer", array_get(array, 0) == new);
  ASSERT("set takes a reference on the new value", new->ref_count == 2);
  ASSERT("set releases the old value", old->ref_count == 1);

  ASSERT("set past length errors", array_set(array, old, 1) == RET_ERR);
  ASSERT("set NULL value errors", array_set(array, NULL, 0) == RET_ERR);
  ASSERT("set on NULL errors", array_set(NULL, old, 0) == RET_ERR);
  ASSERT("set on non-array errors", array_set(old, new, 0) == RET_ERR);

  /*
  self-set: inc before dec keeps the element alive even when the array
  holds the only other reference; the count must end where it started
  */
  ASSERT("self-set succeeds", array_set(array, new, 0) == RET_OK);
  ASSERT("self-set leaves the count unchanged", new->ref_count == 2);

  ref_count_dec(array);
  ref_count_dec(old);
  ref_count_dec(new);
}

static void test_duplicate_child(void)
{
  Object *array;
  Object *child;

  array = new_array(2);
  child = new_integer(1);

  array_append(array, child);
  array_append(array, child);
  ASSERT("each slot takes its own reference", child->ref_count == 3);

  ref_count_dec(array);
  /*
  teardown must NOT dedupe decrements: two slots give back two references
  */
  ASSERT("teardown releases one reference per slot", child->ref_count == 1);
  ref_count_dec(child);
}

static void test_array_contains(void)
{
  Object *array;
  Object *contained;
  Object *equal_value;

  array = new_array(2);
  contained = new_integer(2);
  equal_value = new_integer(2);
  array_append(array, contained);

  ASSERT("contains finds stored pointer", array_contains(array, contained) == TRUE);
  ASSERT("contains compares identity", array_contains(array, equal_value) == FALSE);
  ASSERT("contains on NULL is FALSE", array_contains(NULL, contained) == FALSE);
  ASSERT("contains NULL value is FALSE", array_contains(array, NULL) == FALSE);
  ASSERT("contains on non-array is FALSE", array_contains(contained, contained) == FALSE);

  ref_count_dec(array);
  ref_count_dec(contained);
  ref_count_dec(equal_value);
}

static void test_object_length(void)
{
  Object *integer;
  Object *string;
  Object *array;

  integer = new_integer(1);
  string = new_string("hello");
  array = new_array(4);
  array_append(array, integer);

  ASSERT("integer length is 1", object_length(integer) == 1);
  ASSERT("string length is strlen", object_length(string) == 5);
  ASSERT("array length is element count", object_length(array) == 1);
  ASSERT("NULL length is 0", object_length(NULL) == 0);

  ref_count_dec(array);
  ref_count_dec(integer);
  ref_count_dec(string);
}

static void test_object_add(void)
{
  Object *a;
  Object *b;
  Object *result;

  a = new_integer(1);
  b = new_float(1.5f);
  result = object_add(a, a);
  ASSERT("int+int is INTEGER", result->type == INTEGER && result->data.as_int == 2);
  ASSERT("add result starts with one reference", result->ref_count == 1);
  ref_count_dec(result);

  result = object_add(a, b);
  ASSERT("int+float is FLOAT", result->type == FLOAT && result->data.as_float == 2.5f);
  ref_count_dec(result);

  result = object_add(b, a);
  ASSERT("float+int is FLOAT", result->type == FLOAT && result->data.as_float == 2.5f);
  ref_count_dec(result);

  result = object_add(b, b);
  ASSERT("float+float is FLOAT", result->type == FLOAT && result->data.as_float == 3.0f);
  ref_count_dec(result);
  ref_count_dec(a);
  ref_count_dec(b);

  a = new_string("hello, ");
  b = new_string("world!");
  result = object_add(a, b);
  ASSERT("string+string concatenates", strcmp(result->data.as_string, "hello, world!") == 0);
  ASSERT("string result starts with one reference", result->ref_count == 1);
  ref_count_dec(result);

  ASSERT("string+int is NULL", object_add(a, NULL) == NULL);
  ref_count_dec(a);
  ref_count_dec(b);

  a = new_integer(1);
  b = new_string("hello");
  ASSERT("int+string is NULL", object_add(a, b) == NULL);
  ASSERT("NULL+int is NULL", object_add(NULL, a) == NULL);
  ASSERT("int+NULL is NULL", object_add(a, NULL) == NULL);
  ref_count_dec(a);
  ref_count_dec(b);
}

static void test_object_add_array(void)
{
  Object *a;
  Object *b;
  Object *result;
  Object *x;
  Object *y;

  a = new_array(1);
  b = new_array(1);
  x = new_integer(1);
  y = new_integer(2);
  array_append(a, x);
  array_append(b, y);

  result = object_add(a, b);
  ASSERT("array+array length is the sum", object_length(result) == 2);
  ASSERT("concatenation preserves order", array_get(result, 0) == x && array_get(result, 1) == y);
  /*
  shallow copy, but every shared child gains a reference
  */
  ASSERT("result takes a reference on a's children", x->ref_count == 3);
  ASSERT("result takes a reference on b's children", y->ref_count == 3);

  ref_count_dec(result);
  ASSERT("releasing the result releases the children", x->ref_count == 2 && y->ref_count == 2);

  ref_count_dec(a);
  ref_count_dec(b);
  ref_count_dec(x);
  ref_count_dec(y);
}

static void test_cycle_leaks(void)
{
  Object *a;
  Object *b;

  /*
  the teaching point: refcounting cannot collect cycles.
  a and b end at ref_count 1 with no external owner - both leak.
  mark_sweep exists to fill exactly this gap.
  */
  a = new_array(1);
  b = new_array(1);

  array_append(a, b);
  ASSERT("a -> b takes a reference", a->ref_count == 1 && b->ref_count == 2);

  array_append(b, a);
  ASSERT("b -> a closes the cycle", a->ref_count == 2 && b->ref_count == 2);

  ref_count_dec(b);
  ASSERT("dropping b leaves it alive via a", b->ref_count == 1);
  ASSERT("dropping b does not touch a", a->ref_count == 2);

  ref_count_dec(a);
  ASSERT("the cycle keeps both at one reference", a->ref_count == 1 && b->ref_count == 1);
}

static void test_self_cycle_teardown(void)
{
  Object *a;

  /*
  an array holding itself: the visited set stops the teardown from
  re-entering the dying object, so the second dec frees it cleanly
  */
  a = new_array(1);
  array_append(a, a);
  ASSERT("self-append takes a reference", a->ref_count == 2);

  ref_count_dec(a);
  ASSERT("self-cycle survives the first dec", a->ref_count == 1);

  ref_count_dec(a); /* frees a; must not recurse forever or double free */
  ASSERT("self-cycle teardown completes", 1);
}

static void test_null_safety(void)
{
  ref_count_dec(NULL); /* must not crash */
  ASSERT("ref_count_dec(NULL) is safe", 1);
}

int main(void)
{
  test_set();
  test_constructors();
  test_append_takes_reference();
  test_array_get_set();
  test_duplicate_child();
  test_array_contains();
  test_object_length();
  test_object_add();
  test_object_add_array();
  test_cycle_leaks();
  test_self_cycle_teardown();
  test_null_safety();

  printf("%d tests, %d failed\n", tests_run, tests_failed);

  return tests_failed > 0 ? 1 : 0;
}
