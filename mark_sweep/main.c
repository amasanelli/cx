#include <stdio.h>
#include "vm.h"
#include "object.h"

/*
int main(int argc, char *argv[])
{
  VirtualMachine *vm;
  Frame *frame1;
  Frame *frame2;
  Object *object1;
  Object *object2;

  vm = new_vm();
  frame1 = vm_new_frame(vm);

  {
    frame2 = vm_new_frame(vm);
    object1 = new_string("hello!");
    frame_reference_object(frame2, object1);

    object2 = new_string(object1->data.as_string);
    frame_reference_object(frame1, object2);
  }

  printf("%lu\n", vm->frames->length);
  printf("%lu\n", vm->objects->length);

  vm_collect_garbage(vm);

  printf("%lu\n", vm->frames->length);
  printf("%lu\n", vm->objects->length);

  vm_frame_free(vm);

  printf("%lu\n", vm->frames->length);
  printf("%lu\n", vm->objects->length);

  vm_collect_garbage(vm);

  printf("%lu\n", vm->frames->length);
  printf("%lu\n", vm->objects->length);

  vm_free(vm);

  return 0;
}
*/

int main(int argc, char *argv[])
{
  VirtualMachine *vm;
  Frame *frame1;
  Frame *frame2;
  Object *object1;
  Object *object2;

  vm = new_vm();
  frame1 = vm_new_frame(vm);

  object1 = new_array(1);
  frame_reference_object(frame1, object1);

  {
    frame2 = vm_new_frame(vm);
    object2 = new_array(1);
    frame_reference_object(frame2, object2);

    array_append(object2, object1);
  }

  array_append(object1, object2);

  printf("%lu\n", vm->frames->length);
  printf("%lu\n", vm->objects->length);

  vm_collect_garbage(vm);

  printf("%lu\n", vm->frames->length);
  printf("%lu\n", vm->objects->length);

  vm_frame_free(vm);

  printf("%lu\n", vm->frames->length);
  printf("%lu\n", vm->objects->length);

  vm_collect_garbage(vm);

  printf("%lu\n", vm->frames->length);
  printf("%lu\n", vm->objects->length);

  printf("%lu\n", object1->data.as_array.length);
  printf("%lu\n", object2->data.as_array.length);

  vm_free(vm);

  return 0;
}
