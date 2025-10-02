#ifndef _OBJECT_
#define _OBJECT_

typedef enum
{
  INTEGER,
  FLOAT
} Type;

typedef union
{
  int INTEGER;
  float FLOAT;
} Data;

typedef struct
{
  Data data;
  Type type;
} Object;

Object *new_integer(int value);
Object *new_float(float value);

#endif
