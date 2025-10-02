#ifndef _OBJECT_
#define _OBJECT_

typedef enum
{
  INTEGER,
  FLOAT,
  STRING
} Type;

typedef union
{
  int as_int;
  float as_float;
  char *as_string;
} Data;

typedef struct
{
  Data data;
  Type type;
} Object;

Object *new_integer(int value);
Object *new_float(float value);
Object *new_string(char *value);

#endif
