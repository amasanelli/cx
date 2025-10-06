#ifndef _OBJECT_NEW_
#define _OBJECT_NEW_

#include <stdlib.h>
#include "object.h"
#include "vm.h"

Object *new_integer(VirtualMachine *vm, int value);
Object *new_float(VirtualMachine *vm, float value);
Object *new_string(VirtualMachine *vm, char *value);
Object *new_array(VirtualMachine *vm, size_t capacity);
Object *object_add(VirtualMachine *vm, Object *a, Object *b);

#endif
