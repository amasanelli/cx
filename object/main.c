#include <stdio.h>
#include <string.h>
#include "object.h"
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

static void test_new_integer(void)
{
  Object *object;

  object = new_integer(42);
  ASSERT("new_integer returns an object", object != NULL);
  ASSERT("new_integer sets type", object->type == INTEGER);
  ASSERT("new_integer stores value", object->data.as_int == 42);
  object_free(object);

  object = new_integer(-7);
  ASSERT("new_integer stores negative value", object->data.as_int == -7);
  object_free(object);
}

static void test_new_float(void)
{
  Object *object;

  object = new_float(2.5f);
  ASSERT("new_float returns an object", object != NULL);
  ASSERT("new_float sets type", object->type == FLOAT);
  ASSERT("new_float stores value", object->data.as_float == 2.5f);
  object_free(object);
}

static void test_new_string(void)
{
  Object *object;
  char source[6];

  strcpy(source, "hello");

  object = new_string(source);
  ASSERT("new_string returns an object", object != NULL);
  ASSERT("new_string sets type", object->type == STRING);
  ASSERT("new_string copies content", strcmp(object->data.as_string, "hello") == 0);

  /*
  the object owns its own buffer: mutating the source must not leak through
  */
  source[0] = 'X';
  ASSERT("new_string buffer is independent", strcmp(object->data.as_string, "hello") == 0);
  object_free(object);

  object = new_string("");
  ASSERT("empty string is valid", object != NULL);
  ASSERT("empty string has length 0", object_length(object) == 0);
  object_free(object);

  ASSERT("new_string(NULL) returns NULL", new_string(NULL) == NULL);
}

static void test_new_array(void)
{
  Object *object;

  object = new_array(2);
  ASSERT("new_array returns an object", object != NULL);
  ASSERT("new_array sets type", object->type == ARRAY);
  ASSERT("new array is empty", object->data.as_array.length == 0);
  ASSERT("new array keeps requested capacity", object->data.as_array.capacity == 2);
  ASSERT("new array allocates elements", object->data.as_array.elements != NULL);
  object_free(object);

  /*
  capacity 0 keeps elements NULL; first append grows via realloc(NULL, ...)
  */
  object = new_array(0);
  ASSERT("new_array(0) returns an object", object != NULL);
  ASSERT("new_array(0) keeps elements NULL", object->data.as_array.elements == NULL);
  object_free(object);
}

static void test_array_append(void)
{
  Object *array;
  Object *values[10];
  Object *integer;
  size_t i;

  for (i = 0; i < 10; i++)
  {
    values[i] = new_integer((int)i);
  }

  array = new_array(1);
  ASSERT("append succeeds", array_append(array, values[0]) == RET_OK);
  ASSERT("append updates length", object_length(array) == 1);

  for (i = 1; i < 10; i++)
  {
    ASSERT("append past capacity succeeds", array_append(array, values[i]) == RET_OK);
  }
  ASSERT("length tracks all appends", object_length(array) == 10);
  ASSERT("capacity grows to MIN_CAPACITY then doubles", array->data.as_array.capacity == MIN_CAPACITY * 2);
  for (i = 0; i < 10; i++)
  {
    ASSERT("elements intact after growth", array_get(array, i) == values[i]);
  }

  ASSERT("append to NULL errors", array_append(NULL, values[0]) == RET_ERR);
  ASSERT("append NULL value errors", array_append(array, NULL) == RET_ERR);

  integer = new_integer(1);
  ASSERT("append to non-array errors", array_append(integer, values[0]) == RET_ERR);
  object_free(integer);

  /*
  append onto a zero-capacity array (elements starts NULL)
  */
  for (i = 0; i < 10; i++)
  {
    object_free(values[i]);
  }
  object_free(array);

  array = new_array(0);
  integer = new_integer(5);
  ASSERT("append to zero-capacity array succeeds", array_append(array, integer) == RET_OK);
  ASSERT("zero-capacity array grows", array->data.as_array.capacity == MIN_CAPACITY);
  ASSERT("element readable after growth from 0", array_get(array, 0) == integer);
  object_free(integer);
  object_free(array);
}

static void test_array_get(void)
{
  Object *array;
  Object *integer;

  array = new_array(2);
  integer = new_integer(1);
  array_append(array, integer);

  ASSERT("get returns the stored pointer", array_get(array, 0) == integer);
  ASSERT("get past length returns NULL", array_get(array, 1) == NULL);
  ASSERT("get on NULL returns NULL", array_get(NULL, 0) == NULL);
  ASSERT("get on non-array returns NULL", array_get(integer, 0) == NULL);

  object_free(integer);
  object_free(array);
}

static void test_array_set(void)
{
  Object *array;
  Object *a;
  Object *b;

  array = new_array(2);
  a = new_integer(1);
  b = new_integer(2);
  array_append(array, a);

  ASSERT("set replaces element", array_set(array, b, 0) == RET_OK);
  ASSERT("set stored the new pointer", array_get(array, 0) == b);

  /*
  bounds follow length, not capacity: index 1 is unused capacity
  */
  ASSERT("set past length errors", array_set(array, a, 1) == RET_ERR);
  ASSERT("set NULL value errors", array_set(array, NULL, 0) == RET_ERR);
  ASSERT("set on NULL errors", array_set(NULL, a, 0) == RET_ERR);
  ASSERT("set on non-array errors", array_set(a, b, 0) == RET_ERR);

  object_free(a);
  object_free(b);
  object_free(array);
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
  /*
  identity, not value equality: a distinct object with the same value misses
  */
  ASSERT("contains compares identity", array_contains(array, equal_value) == FALSE);
  ASSERT("contains on NULL is FALSE", array_contains(NULL, contained) == FALSE);
  ASSERT("contains NULL value is FALSE", array_contains(array, NULL) == FALSE);
  ASSERT("contains on non-array is FALSE", array_contains(contained, contained) == FALSE);

  object_free(contained);
  object_free(equal_value);
  object_free(array);
}

static void test_object_length(void)
{
  Object *integer;
  Object *floating;
  Object *string;
  Object *array;

  integer = new_integer(1);
  floating = new_float(1.0f);
  string = new_string("hello");
  array = new_array(4);
  array_append(array, integer);
  array_append(array, floating);

  ASSERT("integer length is 1", object_length(integer) == 1);
  ASSERT("float length is 1", object_length(floating) == 1);
  ASSERT("string length is strlen", object_length(string) == 5);
  ASSERT("array length is element count", object_length(array) == 2);
  ASSERT("NULL length is 0", object_length(NULL) == 0);

  object_free(integer);
  object_free(floating);
  object_free(string);
  object_free(array);
}

static void test_object_add_numeric(void)
{
  Object *a;
  Object *b;
  Object *result;

  a = new_integer(1);
  b = new_integer(2);
  result = object_add(a, b);
  ASSERT("int+int is INTEGER", result->type == INTEGER);
  ASSERT("int+int sums", result->data.as_int == 3);
  object_free(result);
  object_free(b);

  b = new_float(1.5f);
  result = object_add(a, b);
  ASSERT("int+float is FLOAT", result->type == FLOAT);
  ASSERT("int+float sums", result->data.as_float == 2.5f);
  object_free(result);

  result = object_add(b, a);
  ASSERT("float+int is FLOAT", result->type == FLOAT);
  ASSERT("float+int sums", result->data.as_float == 2.5f);
  object_free(result);
  object_free(a);

  a = new_float(1.0f);
  result = object_add(a, b);
  ASSERT("float+float is FLOAT", result->type == FLOAT);
  ASSERT("float+float sums", result->data.as_float == 2.5f);
  object_free(result);
  object_free(a);
  object_free(b);
}

static void test_object_add_string(void)
{
  Object *a;
  Object *b;
  Object *result;

  a = new_string("hello, ");
  b = new_string("world!");
  result = object_add(a, b);
  ASSERT("string+string is STRING", result->type == STRING);
  ASSERT("string+string concatenates", strcmp(result->data.as_string, "hello, world!") == 0);
  object_free(result);
  object_free(a);
  object_free(b);

  a = new_string("");
  b = new_string("");
  result = object_add(a, b);
  ASSERT("empty+empty is valid", result != NULL);
  ASSERT("empty+empty is empty", object_length(result) == 0);
  object_free(result);
  object_free(a);
  object_free(b);
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
  ASSERT("array+array is ARRAY", result->type == ARRAY);
  ASSERT("array+array length is the sum", object_length(result) == 2);
  ASSERT("concatenation preserves order", array_get(result, 0) == x);
  ASSERT("b elements follow a elements", array_get(result, 1) == y);
  /*
  shallow copy: the result shares element pointers with its sources
  */
  ASSERT("array+array is shallow", array_get(result, 0) == array_get(a, 0));
  object_free(result);
  object_free(a);
  object_free(b);

  a = new_array(0);
  b = new_array(0);
  result = object_add(a, b);
  ASSERT("empty+empty arrays is valid", result != NULL);
  ASSERT("empty+empty arrays is empty", object_length(result) == 0);
  object_free(result);
  object_free(a);
  object_free(b);

  object_free(x);
  object_free(y);
}

static void test_object_add_mismatch(void)
{
  Object *integer;
  Object *string;
  Object *array;

  integer = new_integer(1);
  string = new_string("hello");
  array = new_array(1);

  ASSERT("int+string is NULL", object_add(integer, string) == NULL);
  ASSERT("string+int is NULL", object_add(string, integer) == NULL);
  ASSERT("string+array is NULL", object_add(string, array) == NULL);
  ASSERT("array+int is NULL", object_add(array, integer) == NULL);
  ASSERT("NULL+int is NULL", object_add(NULL, integer) == NULL);
  ASSERT("int+NULL is NULL", object_add(integer, NULL) == NULL);

  object_free(integer);
  object_free(string);
  object_free(array);
}

static void test_object_free_null(void)
{
  object_free(NULL); /* must not crash */
  ASSERT("object_free(NULL) is safe", 1);
}

int main(void)
{
  test_new_integer();
  test_new_float();
  test_new_string();
  test_new_array();
  test_array_append();
  test_array_get();
  test_array_set();
  test_array_contains();
  test_object_length();
  test_object_add_numeric();
  test_object_add_string();
  test_object_add_array();
  test_object_add_mismatch();
  test_object_free_null();

  printf("%d tests, %d failed\n", tests_run, tests_failed);

  return tests_failed > 0 ? 1 : 0;
}
