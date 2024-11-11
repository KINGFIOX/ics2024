#include <stdio.h>

#include "inst.h"

void add(int rd, int w, int rs) {
  printf("rd = %d, w = %d, rs = %d\n", rd, w, rs);
  printf("Rr(rd, w) = %x, Rr(rs, w) = %x\n", Rr(rd, w), Rr(rs, w));
  reg_write(rd, w, Rr(rd, w) + Rr(rs, w));
}
