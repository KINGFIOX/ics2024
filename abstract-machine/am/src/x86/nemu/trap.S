#----|------------entry------------|---irq id---|-----handler-----|
.globl __am_vecsys;    __am_vecsys: pushl $0x80; jmp __am_asm_trap
.globl __am_vectrap;  __am_vectrap: pushl $0x81; jmp __am_asm_trap
.globl __am_irq0;        __am_irq0: pushl   $32; jmp __am_asm_trap
.globl __am_vecnull;  __am_vecnull: pushl   $-1; jmp __am_asm_trap


__am_asm_trap:
  pushal

  pushl $0

  pushl %esp
  call __am_irq_handle # def in abstract-machine/am/src/x86/nemu/cte.c

  addl $4, %esp

  addl $4, %esp
  popal # push all general register, except %esp
  addl $4, %esp

  iret
