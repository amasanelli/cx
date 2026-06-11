#ifndef VM_H
#define VM_H

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
  VirtualMachine *vm;
} Frame;

VirtualMachine *new_vm(void);
void vm_free(VirtualMachine *vm);

Frame *vm_new_frame(VirtualMachine *vm);
void vm_frame_free(VirtualMachine *vm);

int frame_reference_object(Frame *frame, Object *object);

void vm_collect_garbage(VirtualMachine *vm);

#endif
