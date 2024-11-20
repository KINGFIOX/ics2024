.section entry, "ax"
.globl _start
.type _start, @function

_start:
  mov $0, %ebp
  mov $_stack_pointer, %esp # set in the linker.ld
  call _trm_init                 # never return
