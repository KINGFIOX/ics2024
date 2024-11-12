
#include "common.h"
#include "inst.h"

word_t add(int w, word_t op1, word_t op2) {
  static const word_t sign_mask = 1 << (sizeof(word_t) * 8 - 1);

  word_t ret = op1 + op2;

  // cf
  if ((uint64_t)ret < (uint64_t)(op1 + op2)) {
    cpu.eflags.cf = 1;
  }

  // pf
  int ones = 0;
  for (int i = 0; i < 8; i++) {
    if (ret & (1 << i)) {
      ones++;
    }
  }
  if (ones % 2 == 1) {
    cpu.eflags.pf = 1;
  }

  // af
  if ((op1 & 0xf) + (op2 & 0xf) > 0xf) {
    cpu.eflags.af = 1;
  }

  // zf
  if (ret == 0) {
    cpu.eflags.zf = 1;
  }

  // sf
  if (ret & sign_mask) {
    cpu.eflags.sf = 1;
  }

  // of
  if ((op1 & sign_mask) == (op2 & sign_mask) && (op1 & sign_mask) != (ret & sign_mask)) {
    cpu.eflags.of = 1;
  }

  return ret;
}
