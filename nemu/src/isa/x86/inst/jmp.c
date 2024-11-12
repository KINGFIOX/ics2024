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
  bool sf = 0 != cpu.eflags.sf;
  bool of = 0 != cpu.eflags.of;
  bool zf = 0 != cpu.eflags.zf;

  if ((sf ^ of) | zf) {
    s->dnpc = s->snpc + SEXT(imm & 0xff, 8);
  }
}
