#include "inst.h"

void cmp(Decode *s, int rd, int w, word_t addr) {
  word_t op1 = Rr(rd, w);
  word_t op2 = vaddr_read(addr, w);
  cpu.eflags.of = 0;
  cpu.eflags.sf = 0;
  cpu.eflags.zf = (op1 == op2);
  cpu.eflags.af = 0;
  cpu.eflags.pf = 0;
  cpu.eflags.cf = op1 < op2;
  assert(cpu.eflags.zf == 1);
  printf("op1 = %d, op2 = %d\n", op1, op2);
}