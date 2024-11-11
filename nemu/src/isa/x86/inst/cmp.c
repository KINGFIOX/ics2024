#include <stdio.h>

#include "inst.h"

void cmp(int rd, int w, word_t addr) {
  word_t op1 = Rr(rd, w);
  word_t op2 = vaddr_read(addr, w);
  cpu.eflags.of = 0;
  cpu.eflags.sf = 0;
  cpu.eflags.zf = (op1 == op2);
  cpu.eflags.af = 0;
  cpu.eflags.pf = 0;
  cpu.eflags.cf = op1 < op2;
}

void cmpb(int w, word_t addr, word_t imm) {
  word_t op1 = vaddr_read(addr, w);
  word_t op2 = imm & 0xff;
  cpu.eflags.of = 0;
  cpu.eflags.sf = 0;
  cpu.eflags.zf = (op1 == op2);
  printf("op1 = %x, op2 = %x\n", op1, op2);
}
