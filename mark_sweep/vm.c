#include "vm.h"
#include "definitions.h"

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

StackFrame *vm_frame_pop(VirtualMachine *vm)
{
  if (vm == NULL)
  {
    return NULL;
  }

  return stack_pop(vm->frames);
}

void mark(VirtualMachine *vm)
{
  int i;
  int j;
  StackFrame *frame = NULL;
  Object *object = NULL;

  for (i = 0; i < vm->frames->length; i++)
  {
    frame = (StackFrame *)vm->frames->data[i];
    for (j = 0; j < frame->references->length; j++)
    {
      object = (Object *)frame->references->data[j];
      object->marked = TRUE;
    }
  }

  return;
}

void trace_mark_object(Stack *gray_objects, Object *object)
{
  if (gray_objects == NULL || object == NULL || object->marked)
  {
    return;
  }

  object->marked = TRUE;

  stack_push(gray_objects, object);

  return;
}

void trace_blacken_object(Stack *gray_objects, Object *object)
{
  int i;

  if (gray_objects == NULL || object == NULL)
  {
    return;
  }

  switch (object->type)
  {
  case ARRAY:
    for (i = 0; i < object_length(object); i++)
    {
      trace_mark_object(gray_objects, object->data.as_array.elements[i]);
    }
    break;
  default:
    break;
  }

  return;
}

void trace(VirtualMachine *vm)
{
  Stack *gray_objects = NULL;
  Object *object = NULL;
  int i;

  if (vm == NULL)
  {
    return;
  }

  gray_objects = new_stack(8);
  if (gray_objects == NULL)
  {
    return;
  }

  for (i = 0; i < vm->objects->length; i++)
  {
    object = (Object *)vm->objects->data[i];
    if (object->marked)
    {
      stack_push(gray_objects, object);
    }
  }

  while (gray_objects->length > 0)
  {
    object = (Object *)stack_pop(gray_objects);
    trace_blacken_object(gray_objects, object);
  }

  stack_free(gray_objects);

  return;
}

void sweep(VirtualMachine *vm)
{
  Object *object = NULL;
  int i;

  if (vm == NULL)
  {
    return;
  }

  for (i = 0; i < vm->objects->length; i++)
  {
    object = (Object *)vm->objects->data[i];
    if (object->marked)
    {
      object->marked = FALSE;
      continue;
    }

    object_free(object);
    vm->objects->data[i] = NULL;
  }

  stack_remove_nulls(vm->objects);

  return;
}

VirtualMachine *new_vm(void)
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
  for (i = 0; i < vm->objects->length; i++)
  {
    object_free(vm->objects->data[i]);
  }
  stack_free(vm->objects);
  free(vm);

  return;
}

int vm_track_object(VirtualMachine *vm, Object *object)
{
  if (vm == NULL || object == NULL)
  {
    return RET_ERR;
  }

  object->marked = FALSE;

  if (stack_push(vm->objects, object) == RET_ERR)
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

int frame_reference_object(StackFrame *frame, Object *object)
{
  if (frame == NULL || object == NULL)
  {
    return RET_ERR;
  }

  if (stack_push(frame->references, object) == RET_ERR)
  {
    return RET_ERR;
  }

  return RET_OK;
}

void vm_collect_garbage(VirtualMachine *vm)
{
  mark(vm);
  trace(vm);
  sweep(vm);
}
