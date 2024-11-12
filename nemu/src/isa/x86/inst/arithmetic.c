

#include <stdio.h>

#include "common.h"
#include "inst.h"

static const word_t sign_mask = 1 << (sizeof(word_t) * 8 - 1);

static const word_t all = -1;

static inline int ones(word_t ret) {
  int ones = 0;
  for (int i = 0; i < 8; i++) {
    if (ret & (1 << i)) {
      ones++;
    }
  }
  return ones;
}

word_t add(int w, word_t op1, word_t op2) {
  word_t ret = op1 + op2;

  // cf
  cpu.eflags.cf = ((uint64_t)(op1 + op2) > (uint64_t)(sign_mask << 1));

  // pf
  cpu.eflags.pf = (ones(ret) % 2 == 1);

  // af
  cpu.eflags.af = ((op1 & 0xf) + (op2 & 0xf) > 0xf);

  // zf
  cpu.eflags.zf = (ret == 0);

  // sf
  cpu.eflags.sf = (ret & sign_mask);

  // of
  cpu.eflags.of = ((op1 & sign_mask) == (op2 & sign_mask) && (op1 & sign_mask) != (ret & sign_mask));

  return ret;
}

word_t sub(int w, word_t op1, word_t op2) { return add(w, op1, -op2); }

void cmp(int w, word_t op1, word_t op2) { sub(w, op1, op2); }

word_t and_(int w, word_t op1, word_t op2) {
  word_t ret = op1 & op2;

  // zf
  cpu.eflags.zf = (ret == 0);

  // sf
  cpu.eflags.sf = (ret & sign_mask);

  // pf
  cpu.eflags.pf = (ones(ret) % 2 == 1);

  // af
  cpu.eflags.af = 0;

  // cf
  cpu.eflags.cf = 0;

  // of
  cpu.eflags.of = 0;

  return ret;
}

void test(int w, word_t op1, word_t op2) { and_(w, op1, op2); }

word_t xor_(int w, word_t op1, word_t op2) {
  printf("w = %d\n", w);
  printf("op1 = 0b%08b, op2 = 0b%08b\n", op1, op2);
  word_t ret = op1 ^ op2;

  // zf
  cpu.eflags.zf = (0 == ret);

  // sf
  cpu.eflags.sf = (ret & sign_mask);

  // pf
  cpu.eflags.pf = (ones(ret) % 2 == 1);

  // cf
  cpu.eflags.cf = 0;

  // af
  cpu.eflags.af = 0;

  // of
  cpu.eflags.of = 0;

  printf("ret = 0b%08b\n", ret);
  return ret;
}

word_t not_(int w, word_t op1) { return xor_(w, op1, all); }

word_t shr(int w, word_t op1, word_t op2) {
  word_t ret = op1 >> op2;

  // TODO: sf, cf, of. 可能有循环移位之类的, 没法确定

  // zf
  cpu.eflags.zf = (ret == 0);

  // pf
  cpu.eflags.pf = (ones(ret) % 2 == 1);

  return ret;
}
