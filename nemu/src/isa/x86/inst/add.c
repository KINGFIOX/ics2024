#include "inst.h"

void add(int rd, int w, int rs) { reg_write(rd, w, Rr(rd, w) + Rr(rs, w)); }
