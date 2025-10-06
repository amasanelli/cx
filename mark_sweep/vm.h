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
int vm_frame_push(VirtualMachine *vm, StackFrame *frame);
int vm_track_object(VirtualMachine *vm, Object *object);
void vm_mark(VirtualMachine *vm);

StackFrame *vm_new_frame(VirtualMachine *vm);
int frame_reference_object(StackFrame *frame, Object *object);
void frame_free(StackFrame *frame);

#endif
