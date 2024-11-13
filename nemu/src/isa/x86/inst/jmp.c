#include "common.h"
#include "cpu/decode.h"
#include "debug.h"
#include "inst.h"

// offset
void callo(Decode* s, int w, word_t imm) {
  assert(4 == w);
  push(w, s->snpc);
  printf("1: dnpc = %x, snpc = %x, imm = %x\n", s->dnpc, s->snpc, imm);
  s->dnpc = s->snpc + imm;
  printf("2: dnpc = %x, snpc = %x, imm = %x\n", s->dnpc, s->snpc, imm);
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

void jcc(Decode* s, word_t imm, uint8_t subcode) {
  bool cond;
  switch (subcode) {
    case 0b0010:  // jb
      cond = cpu.eflags.cf != 0;
      break;
    case 0b0100:  // je
      cond = cpu.eflags.zf != 0;
      break;
    case 0b0101:  // jne
      cond = cpu.eflags.zf == 0;
      break;
    case 0b0110:  // jbe
      cond = !!(!!cpu.eflags.cf | !!cpu.eflags.zf);
      break;
    case 0b1000:  // js
      cond = !!cpu.eflags.sf;
      break;
    case 0b1101:  // jge
      cond = !(!!cpu.eflags.sf ^ !!cpu.eflags.of);
      break;
    case 0b1110:  // jle
      cond = (!!cpu.eflags.sf ^ !!cpu.eflags.of) | !!cpu.eflags.zf;
      break;
    default:
      false;
      Assert(false, "subcode = %d", subcode);
  }
  if (cond) {
    s->dnpc = s->snpc + imm;
  }
}

// offset
void jmpo(Decode* s, word_t imm) { s->dnpc = s->snpc + SEXT(imm & 0xff, 8); }

// absolute
void jmpa(Decode* s, word_t imm) { s->dnpc = imm; }
