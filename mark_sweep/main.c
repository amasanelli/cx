#include <stdio.h>
#include "vm.h"
#include "object.h"

int main(int argc, char *argv[])
{
  VirtualMachine *vm;
  Frame *frame;
  Object *object;

  vm = new_vm();
  frame = vm_new_frame(vm);
  object = new_string("hello!");
  frame_reference_object(frame, object);

  vm_collect_garbage(vm);

  vm_frame_free(vm);

  vm_collect_garbage(vm);

  vm_free(vm);

  return 0;
}
