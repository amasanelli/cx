#ifndef _VM_
#define _VM_

#include "stack.h"

typedef struct
{
  Stack *frames;
  Stack *objects;
} VM;

VM *new_vm();
void vm_free(VM *vm);

#endif
