#ifndef _VM_
#define _VM_

#include "stack.h"

typedef struct
{
  Stack *frames;
  Stack *objects;
} VirtualMachine;

typedef struct
{
  Stack *references;
} StackFrame;

VirtualMachine *new_vm();
void vm_free(VirtualMachine *vm);
int vm_frame_push(VirtualMachine *vm, StackFrame *frame);
StackFrame *vm_new_frame(VirtualMachine *vm);
void frame_free(StackFrame *frame);

#endif
