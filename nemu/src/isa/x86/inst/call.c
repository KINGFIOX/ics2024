#include "inst.h"

void call(Decode* s, int w, word_t imm) {
  assert(4 == w);
  push(w, s->snpc);
  s->dnpc = s->snpc + imm;
}