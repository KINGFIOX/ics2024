#include "inst.h"
#include "isa-def.h"
#include "memory/vaddr.h"

void push(int width, word_t data) {
  assert(width == 1 || width == 2 || width == 4);

  vaddr_t vaddr = reg_read(R_ESP, 4);  // get %esp, %esp is 4 bytes
  vaddr -= width;
  reg_write(R_ESP, 4, vaddr);
  vaddr_write(vaddr, width, data);
}

word_t pop(int width) {
  assert(width == 1 || width == 2 || width == 4);

  vaddr_t vaddr = reg_read(R_ESP, 4);
  word_t data = vaddr_read(vaddr, width);
  vaddr += width;
  reg_write(R_ESP, 4, vaddr);
  return data;
}