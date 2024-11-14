
#include "common.h"
#include "cpu/decode.h"
#include "debug.h"
#include "inst.h"

// offset
void callo(Decode* s, int w, word_t imm) {
  assert(4 == w);
  push(w, s->snpc);
  s->dnpc = s->snpc + imm;
#ifdef CONFIG_FTRACE
  extern void push_call_stack(vaddr_t, vaddr_t);
  push_call_stack(s->dnpc, cpu.pc);
#endif
}

// offset
void calla(Decode* s, int w, word_t imm) {
  assert(4 == w);
  push(w, s->snpc);
  s->dnpc = imm;
#ifdef CONFIG_FTRACE
  extern void push_call_stack(vaddr_t, vaddr_t);
  push_call_stack(s->dnpc, cpu.pc);
#endif
}

void ret(Decode* s, int w) {
  s->dnpc = pop(4);
#ifdef CONFIG_FTRACE
  extern void pop_call_stack(vaddr_t);
  pop_call_stack(cpu.pc);
#endif
}

void jcc(Decode* s, word_t imm, uint8_t subcode) {
  bool cond;
  bool cf = !!cpu.eflags.cf;
  bool zf = !!cpu.eflags.zf;
  bool sf = !!cpu.eflags.sf;
  bool of = !!cpu.eflags.of;
  switch (subcode) {
    case 0b0010:  // jb
      cond = cf;
      break;
    case 0b0011:  // jae
      cond = !cf;
      break;
    case 0b0100:  // je
      cond = zf;
      break;
    case 0b0101:  // jne
      cond = !zf;
      break;
    case 0b0110:  // jbe
      cond = cf | zf;
      break;
    case 0b0111:  // ja
      cond = !(cf | zf);
      break;
    case 0b1000:  // js
      cond = sf;
      break;
    case 0b1001:  // jns
      cond = !sf;
      break;
    case 0b1100:  // jl
      cond = sf ^ of;
      break;
    case 0b1101:  // jge
      cond = !(sf ^ of);
      break;
    case 0b1110:  // jle
      cond = (sf ^ of) | zf;
      break;
    case 0b1111:  // jg
      cond = !((sf ^ of) | zf);
      break;
    default:
      false;
      Assert(false, "subcode = 0b%04b", subcode);
  }
  if (cond) {
    s->dnpc = s->snpc + imm;
  }
}

// offset
void jmpo(Decode* s, word_t imm) { s->dnpc = s->snpc + imm; }

// absolute
void jmpa(Decode* s, word_t imm) { s->dnpc = imm; }
