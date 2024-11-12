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
    s->dnpc = s->snpc + SEXT(imm & 0xff, 8);
  }
}

void jb(Decode* s, word_t imm) {
  if (cpu.eflags.cf != 0) {
    s->dnpc = s->snpc + SEXT(imm & 0xff, 8);
  }
}

void jne(Decode* s, word_t imm) {
  if (cpu.eflags.zf == 0) {
    s->dnpc = s->snpc + SEXT(imm & 0xff, 8);
  }
}

void jmp(Decode* s, word_t imm) { s->dnpc = s->snpc + SEXT(imm & 0xff, 8); }

void jle(Decode* s, word_t imm) {
  // (sf ^ of) | zf
  int sf = !!cpu.eflags.sf;
  int of = !!cpu.eflags.of;
  int zf = !!cpu.eflags.zf;

  int cond = (sf ^ of) | zf;

  printf("cond = %d\n", cond);
  printf("sf = %d, of = %d, zf = %d\n", sf, of, zf);
  printf("(sf ^ of) = %d, zf = %d\n", (sf ^ of), zf);

  if (cond) {
    s->dnpc = s->snpc + SEXT(imm & 0xff, 8);
  }
}
