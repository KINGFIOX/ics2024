#include "debug.h"
#include "inst.h"
#include "isa-def.h"
#include "memory/vaddr.h"

void push(int width, word_t data) {
  assert(width == 1 || width == 2 || width == 4);

  vaddr_t vaddr = reg_read(R_ESP, 4);  // get %esp, %esp is 4 bytes
  vaddr -= width;
  reg_write(R_ESP, 4, vaddr);
  vaddr_write(vaddr, width, data);

  Assert(vaddr_read(reg_read(R_ESP, 4), width) == data, "push error");
}

word_t pop(int width) {
  assert(width == 1 || width == 2 || width == 4);

  vaddr_t vaddr = reg_read(R_ESP, 4);
  word_t data = vaddr_read(vaddr, width);
  vaddr += width;
  reg_write(R_ESP, 4, vaddr);
  return data;
}

void leave(void) {
  reg_write(R_ESP, 4, reg_read(R_EBP, 4));
  reg_write(R_EBP, 4, pop(4));
}
