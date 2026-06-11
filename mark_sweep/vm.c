#include <stdio.h>
#include "vm.h"
#include "definitions.h"

void frame_free(Frame *frame)
{
  if (frame == NULL)
  {
    return;
  }

  stack_free(frame->references);
  free(frame);

  return;
}

int vm_frame_push(VirtualMachine *vm, Frame *frame)
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

Frame *vm_frame_pop(VirtualMachine *vm)
{
  if (vm == NULL)
  {
    return NULL;
  }

  return stack_pop(vm->frames);
}

void mark(VirtualMachine *vm)
{
  size_t i;
  size_t j;
  Frame *frame = NULL;
  Object *object = NULL;

  if (vm == NULL)
  {
    return;
  }

  for (i = 0; i < vm->frames->length; i++)
  {
    frame = (Frame *)vm->frames->data[i];
    for (j = 0; j < frame->references->length; j++)
    {
      object = (Object *)frame->references->data[j];
      object->marked = TRUE;
    }
  }

  return;
}

int trace(VirtualMachine *vm)
{
  Stack *gray_objects = NULL;
  Object *object = NULL;
  Object *child = NULL;
  size_t i;
  size_t length;

  if (vm == NULL)
  {
    return RET_ERR;
  }

  gray_objects = new_stack(8);
  if (gray_objects == NULL)
  {
    return RET_ERR;
  }

  for (i = 0; i < vm->objects->length; i++)
  {
    object = (Object *)vm->objects->data[i];
    if (object->marked)
    {
      if (stack_push(gray_objects, object) == RET_ERR)
      {
        stack_free(gray_objects);
        return RET_ERR;
      }
    }
  }

  while (gray_objects->length > 0)
  {
    object = (Object *)stack_pop(gray_objects);
    if (object->type != ARRAY)
    {
      continue;
    }

    /*
    type checked above: read length directly, skip object_length dispatch
    */
    length = object->data.as_array.length;
    for (i = 0; i < length; i++)
    {
      child = (Object *)object->data.as_array.elements[i];
      if (child == NULL || child->marked)
      {
        continue;
      }

      child->marked = TRUE;
      if (stack_push(gray_objects, child) == RET_ERR)
      {
        stack_free(gray_objects);
        return RET_ERR;
      }
    }
  }

  stack_free(gray_objects);

  return RET_OK;
}

void sweep(VirtualMachine *vm)
{
  Object *object = NULL;
  size_t i;
  size_t survivors = 0;

  if (vm == NULL)
  {
    return;
  }

  /*
  free and compact in one pass: survivors slide down to the write index
  */
  for (i = 0; i < vm->objects->length; i++)
  {
    object = (Object *)vm->objects->data[i];
    if (object->marked)
    {
      object->marked = FALSE;
      vm->objects->data[survivors++] = object;
      continue;
    }

    object_free(object);
  }

  /*
  keep dead slots NULL so stale pointers never linger after a sweep
  */
  for (i = survivors; i < vm->objects->length; i++)
  {
    vm->objects->data[i] = NULL;
  }

  vm->objects->length = survivors;

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
    stack_free(frames);
    free(vm);
    return NULL;
  }

  vm->frames = frames;
  vm->objects = objects;

  return vm;
}

void vm_free(VirtualMachine *vm)
{
  size_t i;

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

int vm_object_tracked(VirtualMachine *vm, Object *object)
{
  size_t i;

  if (vm == NULL || object == NULL)
  {
    return FALSE;
  }

  for (i = 0; i < vm->objects->length; i++)
  {
    if (vm->objects->data[i] == object)
    {
      return TRUE;
    }
  }

  return FALSE;
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

Frame *vm_new_frame(VirtualMachine *vm)
{
  Frame *frame = NULL;
  Stack *references = NULL;

  if (vm == NULL)
  {
    return NULL;
  }

  frame = (Frame *)calloc(1, sizeof(Frame));
  if (frame == NULL)
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
  frame->vm = vm;

  if (vm_frame_push(vm, frame) == RET_ERR)
  {
    stack_free(references);
    free(frame);
    return NULL;
  }

  return frame;
}

int frame_reference_object(Frame *frame, Object *object)
{
  int tracked;

  if (frame == NULL || object == NULL)
  {
    return RET_ERR;
  }

  /*
  an object referenced more than once (same frame or another frame) must
  stay tracked exactly once: duplicate slots would make sweep and vm_free
  free it twice
  */
  tracked = vm_object_tracked(frame->vm, object);

  if (!tracked && vm_track_object(frame->vm, object) == RET_ERR)
  {
    return RET_ERR;
  }

  if (stack_push(frame->references, object) == RET_ERR)
  {
    /*
    unwind only what this call pushed: a previously tracked object stays
    */
    if (!tracked)
    {
      stack_pop(frame->vm->objects);
    }
    return RET_ERR;
  }

  return RET_OK;
}

void vm_frame_free(VirtualMachine *vm)
{
  Frame *frame = NULL;

  if (vm == NULL)
  {
    return;
  }

  frame = vm_frame_pop(vm);

  if (frame == NULL)
  {
    return;
  }

  frame_free(frame);

  return;
}

void vm_collect_garbage(VirtualMachine *vm)
{
  if (vm == NULL)
  {
    return;
  }

  mark(vm);
  /*
  if tracing fails, sweeping would free reachable objects
  */
  if (trace(vm) == RET_ERR)
  {
    return;
  }
  sweep(vm);
}
