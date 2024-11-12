#include <stdio.h>

#include "common.h"
#include "inst.h"
#include "macro.h"

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
  assert(4 == w || 2 == w || 1 == w);
  if (1 == w) {
    op1 = (int8_t)op1;
    op2 = (int8_t)op2;
  } else if (2 == w) {
    op1 = (int16_t)op1;
    op2 = (int16_t)op2;
  } else if (4 == w) {
    op1 = (int32_t)op1;
    op2 = (int32_t)op2;
  }

  uint64_t ret_u64 = op1 + op2;

  word_t ret = (word_t)ret_u64;

  int sign_mask = (1 << 31);

  // cf
  cpu.eflags.cf = (ret_u64 > UINT32_MAX);

  // pf
  cpu.eflags.pf = (1 == ones(ret) % 2);

  // zf
  cpu.eflags.zf = (0 == ret);

  // sf
  cpu.eflags.sf = !!(ret & sign_mask);

  // of
  cpu.eflags.of = ((op1 & sign_mask) == (op2 & sign_mask) && (op1 & sign_mask) != (ret & sign_mask));

  return ret;
}

word_t sub(int w, word_t op1, word_t op2) {
  assert(4 == w || 2 == w || 1 == w);
  if (1 == w) {
    op1 = (int8_t)op1;
    op2 = (int8_t)op2;
  } else if (2 == w) {
    op1 = (int16_t)op1;
    op2 = (int16_t)op2;
  } else if (4 == w) {
    op1 = (int32_t)op1;
    op2 = (int32_t)op2;
  }
  word_t neg = -op2;
  return add(w, op1, neg);
}

void cmp(int w, word_t op1, word_t op2) {
  assert(4 == w || 2 == w || 1 == w);
  if (1 == w) {
    op1 = (int8_t)op1;
    op2 = (int8_t)op2;
  } else if (2 == w) {
    op1 = (int16_t)op1;
    op2 = (int16_t)op2;
  } else if (4 == w) {
    op1 = (int32_t)op1;
    op2 = (int32_t)op2;
  }

  sub(w, op1, op2);

  // if (op1 == op2) {
  //   assert(cpu.eflags.zf);
  // }
  // if (op1 < op2) {
  //   assert(cpu.eflags.cf);
  // }
}

word_t and_(int w, word_t op1, word_t op2) {
  int sign_mask = (1 << (w * 8 - 1));

  word_t ret = op1 & op2;

  // zf
  cpu.eflags.zf = (ret == 0);

  // sf
  cpu.eflags.sf = (ret & sign_mask);

  // pf
  cpu.eflags.pf = (ones(ret) % 2 == 1);

  // cf
  cpu.eflags.cf = 0;

  // of
  cpu.eflags.of = 0;

  return ret;
}

void test(int w, word_t op1, word_t op2) { and_(w, op1, op2); }

word_t xor_(int w, word_t op1, word_t op2) {
  int sign_mask = (1 << (w * 8 - 1));

  word_t ret = op1 ^ op2;

  // zf
  cpu.eflags.zf = (0 == ret);

  // sf
  cpu.eflags.sf = (ret & sign_mask);

  // pf
  cpu.eflags.pf = (1 == ones(ret) % 2);

  // cf
  cpu.eflags.cf = 0;

  // of
  cpu.eflags.of = 0;

  return ret;
}

word_t not_(int w, word_t op1) { return xor_(w, op1, all); }

word_t shr(int w, word_t op1, word_t op2) {
  word_t ret = op1 >> op2;

  // TODO: sf, cf, of. 可能有循环移位之类的, 没法确定

  // zf
  cpu.eflags.zf = (0 == ret);

  // pf
  cpu.eflags.pf = (1 == ones(ret) % 2);

  return ret;
}

word_t imul(int w, word_t op1, word_t op2) {
  assert(w == 4);
  int sign_mask = (1 << (w * 8 - 1));

  uint64_t ret = op1 * op2;

  // of, cf
  if (ret > UINT32_MAX) {
    cpu.eflags.cf = 1;
    cpu.eflags.of = 1;
  } else {
    cpu.eflags.cf = 0;
    cpu.eflags.of = 0;
  }

  // pf
  cpu.eflags.pf = (1 == ones(ret) % 2);

  // zf
  cpu.eflags.zf = (0 == ret);

  // sf
  cpu.eflags.sf = (ret & sign_mask);

  // af, do nothing

  return (word_t)ret;
}
