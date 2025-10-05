#include "vm.h"
#include "definitions.h"

VirtualMachine *new_vm()
{
  VirtualMachine *vm = NULL;
  Stack *frames = NULL;
  Stack *objects = NULL;

  vm = (VirtualMachine *)calloc(1, sizeof(VirtualMachine));
  if (vm == NULL)
  {
    return NULL;
  }

  frames = new_stack(8);
  if (frames == NULL)
  {
    free(vm);
    return NULL;
  }

  objects = new_stack(8);
  if (objects == NULL)
  {
    free(frames);
    free(vm);
    return NULL;
  }

  vm->frames = frames;
  vm->objects = objects;

  return vm;
}

void vm_free(VirtualMachine *vm)
{
  int i;

  if (vm == NULL)
  {
    return;
  }

  for (i = 0; i < vm->frames->length; i++)
  {
    frame_free(vm->frames->data[i]);
  }
  stack_free(vm->frames);
  stack_free(vm->objects);
  free(vm);

  return;
}

int vm_frame_push(VirtualMachine *vm, StackFrame *frame)
{
  if (vm == NULL || frame == NULL)
  {
    return RET_ERR;
  }

  if (stack_push(vm->frames, frame) == RET_ERR)
  {
    return RET_ERR;
  }

  return RET_OK;
}

StackFrame *vm_new_frame(VirtualMachine *vm)
{
  StackFrame *frame = NULL;
  Stack *references = NULL;

  if (vm == NULL)
  {
    return NULL;
  }

  frame = (StackFrame *)calloc(1, sizeof(StackFrame));
  if (vm == NULL)
  {
    return NULL;
  }

  references = new_stack(8);
  if (references == NULL)
  {
    free(frame);
    return NULL;
  }

  frame->references = references;

  if (vm_frame_push(vm, frame) == RET_ERR)
  {
    stack_free(references);
    free(frame);
    return NULL;
  }

  return frame;
}

void frame_free(StackFrame *frame)
{
  if (frame == NULL)
  {
    return;
  }

  stack_free(frame->references);
  free(frame);

  return;
}