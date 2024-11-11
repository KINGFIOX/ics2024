#include <stdio.h>

#include "common.h"
#include "cpu/decode.h"
#include "inst.h"

void call(Decode* s, int w, word_t imm) {
  assert(4 == w);
  push(w, s->snpc);
  s->dnpc = s->snpc + imm;
}

void ret(Decode* s, int w) {
  assert(4 == w);
  s->dnpc = pop(w);
}

void je(Decode* s, word_t imm) {
  if (cpu.eflags.zf != 0) {
    s->dnpc = s->snpc + SEXT(imm & 0xff, 2 * 8);
    printf("s->snpc: 0x%x, s->dnpc: 0x%x\n", s->snpc, s->dnpc);
  }
}
