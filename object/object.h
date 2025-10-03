#ifndef _OBJECT_
#define _OBJECT_

typedef struct _object Object;

typedef enum
{
  INTEGER,
  FLOAT,
  STRING,
  ARRAY
} Type;

typedef struct
{
  size_t capacity;
  size_t length;
  Object **elements;
} Array;

typedef union
{
  int as_int;
  float as_float;
  char *as_string;
  Array as_array;
} Data;

struct _object
{
  Data data;
  Type type;
};

Object *new_integer(int value);
Object *new_float(float value);
Object *new_string(char *value);
Object *new_array(size_t capacity);
int array_set(Object *object, Object *value, size_t index);
Object *array_get(Object *object, size_t index);
int array_append(Object *object, Object *value);
size_t object_length(Object *object);
Object *object_add(Object *a, Object *b);
int array_contains(Object *object, Object *value);

#endif
