#ifndef _VM_
#define _VM_

#include "stack.h"
#include "object.h"

typedef struct
{
  Stack *frames;
  Stack *objects;
} VirtualMachine;

typedef struct
{
  Stack *references;
} StackFrame;

VirtualMachine *new_vm(void);
void vm_free(VirtualMachine *vm);
int vm_track_object(VirtualMachine *vm, Object *object);

StackFrame *vm_new_frame(VirtualMachine *vm);
int frame_reference_object(StackFrame *frame, Object *object);

void mark(VirtualMachine *vm);
void trace(VirtualMachine *vm);

#endif
