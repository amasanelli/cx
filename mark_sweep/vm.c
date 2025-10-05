#include "vm.h"

VM *new_vm()
{
  VM *vm = NULL;
  Stack *frames = NULL;
  Stack *objects = NULL;

  vm = (VM *)calloc(1, sizeof(VM));
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
    free(vm);
    free(frames);
    return NULL;
  }

  vm->frames = frames;
  vm->objects = objects;

  return vm;
}

void vm_free(VM *vm)
{
  if (vm == NULL)
  {
    return;
  }

  stack_free(vm->frames);
  stack_free(vm->objects);
  free(vm);
}
