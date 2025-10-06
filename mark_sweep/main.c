#include <stdio.h>
#include "vm.h"
#include "object.h"

int main(int argc, char *argv[])
{
  VirtualMachine *vm;
  Frame *frame1;
  Frame *frame2;
  Object *object;

  vm = new_vm();
  frame1 = vm_new_frame(vm);
  frame2 = vm_new_frame(vm);
  object = new_string("hello!");
  frame_reference_object(frame2, object);
  frame_reference_object(frame1, object);
  printf("%s\n", object->data.as_string);

  vm_collect_garbage(vm);
  printf("%s\n", object->data.as_string);

  vm_frame_free(vm);
  printf("%s\n", object->data.as_string);

  vm_collect_garbage(vm);
  printf("%s", object->data.as_string);

  return 0;
}
