#include "common.h"
#include "cpu/decode.h"
#include "inst.h"

// offset
void callo(Decode* s, int w, word_t imm) {
  assert(4 == w);
  push(w, s->snpc);
  s->dnpc = s->snpc + imm;
}

// offset
void calla(Decode* s, int w, word_t imm) {
  assert(4 == w);
  push(w, s->snpc);
  s->dnpc = imm;
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

// offset
void jmpo(Decode* s, word_t imm) { s->dnpc = s->snpc + SEXT(imm & 0xff, 8); }

// absolute
void jmpa(Decode* s, word_t imm) { s->dnpc = imm; }

void jle(Decode* s, word_t imm) {
  // (sf ^ of) | zf
  int sf = !!cpu.eflags.sf;
  int of = !!cpu.eflags.of;
  int zf = !!cpu.eflags.zf;
  int cond = (sf ^ of) | zf;

  if (cond) {
    s->dnpc = s->snpc + SEXT(imm & 0xff, 8);
  }
}

void jbe(Decode* s, word_t imm) {
  // cf | zf
  int cf = !!cpu.eflags.cf;
  int zf = !!cpu.eflags.zf;
  int cond = !!(cf | zf);

  if (cond) {
    s->dnpc = s->snpc + SEXT(imm & 0xff, 8);
  }
}

void js(Decode* s, word_t imm) {
  int sf = !!cpu.eflags.sf;
  int cond = sf;

  if (cond) {
    s->dnpc = s->snpc + SEXT(imm & 0xff, 8);
  }
}

void jge(Decode* s, word_t imm) {
  int sf = !!cpu.eflags.sf;
  int of = !!cpu.eflags.of;
  int cond = !(sf ^ of);

  if (cond) {
    s->dnpc = s->snpc + SEXT(imm & 0xff, 8);
  }
}
