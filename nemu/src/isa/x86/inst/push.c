#include "inst.h"
#include "isa-def.h"
#include "memory/vaddr.h"

void push(int width, word_t data) {
  assert(width == 1 || width == 2 || width == 4);

  vaddr_t vaddr = reg_read(R_ESP, 4);
  reg_write(R_ESP, 4, vaddr - width);
  vaddr_write(vaddr, width, data);
}