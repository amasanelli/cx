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

#endif
