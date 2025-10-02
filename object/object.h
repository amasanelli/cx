#ifndef _OBJECT_
#define _OBJECT_

typedef enum
{
  INTEGER
} Type;

typedef union
{
  int integer;
} Data;

typedef struct
{
  Data data;
  Type type;
} Object;

Object *new_integer(int value);

#endif
