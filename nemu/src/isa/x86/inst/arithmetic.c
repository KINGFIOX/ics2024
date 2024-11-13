#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "inst.h"

static inline int ones(word_t ret) {
  int ones = 0;
  for (int i = 0; i < 8; i++) {
    if (ret & (1 << i)) {
      ones++;
    }
  }
  return ones;
}

word_t add(int w, word_t op1_, word_t op2_, bool adc) {
  assert(4 == w || 2 == w || 1 == w);
  uint64_t op1 = 0;
  uint64_t op2 = 0;
  if (1 == w) {
    op1 = (uint8_t)op1_;
    op2 = (uint8_t)op2_;
  } else if (2 == w) {
    op1 = (uint16_t)op1_;
    op2 = (uint16_t)op2_;
  } else if (4 == w) {
    op1 = (uint32_t)op1_;
    op2 = (uint32_t)op2_;
  }

  uint64_t w_u64 = w;  // NOTE: 多少是对 c 语言的字面量类型感到难绷了
  const uint64_t sign_mask = (uint64_t)1 << (w_u64 * 8 - 1);
  const uint64_t mask = ((uint64_t)1 << (w_u64 * 8)) - 1;

  uint64_t ret_u64;
  if (adc) {
    ret_u64 = op1 + op2 + !!cpu.eflags.cf;
  } else {
    ret_u64 = op1 + op2;
  }
  bool op1_sign = !!(op1 & sign_mask);
  bool op2_sign = !!(op2 & sign_mask);
  bool ret_sign = !!(ret_u64 & sign_mask);

  cpu.eflags.cf = !!(ret_u64 & (~mask));            // cf
  cpu.eflags.pf = (1 == ones(ret_u64 & mask) % 2);  // pf
  cpu.eflags.zf = !(ret_u64 & mask);                // zf
  cpu.eflags.sf = ret_sign;                         // sf

  // + plus + == - || - plus - == +
  cpu.eflags.of = ((!op1_sign) && (!op2_sign) && (ret_sign)) || ((op1_sign) && (op2_sign) && (!ret_sign));

  return ret_u64;
}

word_t sub(int w, word_t op1_, word_t op2_, bool sbb) {
  assert(4 == w || 2 == w || 1 == w);

  uint64_t op1 = 0;
  uint64_t op2 = 0;
  if (1 == w) {
    op1 = (uint8_t)op1_;
    op2 = (uint8_t)op2_;
  } else if (2 == w) {
    op1 = (uint16_t)op1_;
    op2 = (uint16_t)op2_;
  } else if (4 == w) {
    op1 = (uint32_t)op1_;
    op2 = (uint32_t)op2_;
  }

  uint64_t w_u64 = w;  // NOTE: 多少是对 c 语言的字面量类型感到难绷了
  const uint64_t sign_mask = (uint64_t)1 << (w_u64 * 8 - 1);
  const uint64_t mask = ((uint64_t)1 << (w_u64 * 8)) - 1;

  uint64_t ret_u64;
  if (sbb) {
    ret_u64 = op1 - op2 - !!cpu.eflags.cf;
  } else {
    ret_u64 = op1 - op2;
  }
  bool op1_sign = !!(op1 & sign_mask);
  bool op2_sign = !!(op2 & sign_mask);
  bool ret_sign = !!(ret_u64 & sign_mask);
  printf("ret_u64 = 0x%08lx, ret_sign = %d, w = %d\n", ret_u64, ret_sign, w);
  printf("sign_mask = 0x%08lx, mask = 0x%08lx\n", sign_mask, mask);

  cpu.eflags.cf = !!(op1 < op2);                    // cf
  cpu.eflags.pf = (1 == ones(ret_u64 & mask) % 2);  // pf
  cpu.eflags.zf = !(ret_u64 & mask);                // zf
  cpu.eflags.sf = ret_sign;                         // sf
  // + minus - == - || - minus + == +
  cpu.eflags.of = ((!op1_sign) && (op2_sign) && (ret_sign)) || ((op1_sign) && (!op2_sign) && (!ret_sign));

  return ret_u64;
}

void cmp(int w, word_t op1_, word_t op2_) {
  assert(4 == w || 2 == w || 1 == w);

  word_t op1 = 0;
  word_t op2 = 0;
  if (1 == w) {
    op1 = (uint8_t)op1_;
    op2 = (uint8_t)op2_;
  } else if (2 == w) {
    op1 = (uint16_t)op1_;
    op2 = (uint16_t)op2_;
  } else if (4 == w) {
    op1 = (uint32_t)op1_;
    op2 = (uint32_t)op2_;
  }

  sub(w, op1, op2, false);

  if (op1 == op2) {
    assert(cpu.eflags.zf);
  }
  //
}

word_t and_(int w, word_t op1, word_t op2) {
  assert(4 == w || 2 == w || 1 == w);
  int sign_mask = (1 << (w * 8 - 1));
  word_t ret = op1 & op2;

  cpu.eflags.zf = !ret;                 // zf
  cpu.eflags.sf = !!(ret & sign_mask);  // sf
  cpu.eflags.pf = !!(ones(ret) % 2);    // pf
  cpu.eflags.cf = 0;                    // cf
  cpu.eflags.of = 0;                    // of

  return ret;
}

void test(int w, word_t op1, word_t op2) {
  assert(4 == w || 2 == w || 1 == w);
  and_(w, op1, op2);
}

word_t xor_(int w, word_t op1, word_t op2) {
  int sign_mask = (1 << (w * 8 - 1));
  word_t ret = op1 ^ op2;

  cpu.eflags.zf = (0 == ret);            // zf
  cpu.eflags.sf = (ret & sign_mask);     // sf
  cpu.eflags.pf = (1 == ones(ret) % 2);  // pf
  cpu.eflags.cf = 0;                     // cf
  cpu.eflags.of = 0;                     // of

  return ret;
}

word_t or_(int w, word_t op1, word_t op2) {
  int sign_mask = (1 << (w * 8 - 1));
  word_t ret = op1 | op2;

  cpu.eflags.zf = (0 == ret);            // zf
  cpu.eflags.sf = (ret & sign_mask);     // sf
  cpu.eflags.pf = (1 == ones(ret) % 2);  // pf
  cpu.eflags.cf = 0;                     // cf
  cpu.eflags.of = 0;                     // of

  return ret;
}

word_t not_(int w, word_t op1) { return xor_(w, op1, all); }

word_t sar(int w, word_t op1, word_t op2) {
  assert(w == 4);
  sword_t ret = (sword_t)op1 >> op2;

  // TODO: sf, cf, of. 可能有循环移位之类的, 没法确定

  cpu.eflags.zf = (0 == ret);            // zf
  cpu.eflags.pf = (1 == ones(ret) % 2);  // pf

  return ret;
}

word_t shl(int w, word_t op1, word_t op2) {
  assert(w == 4);
  word_t ret = op1 << op2;

  // TODO: sf, cf, of. 可能有循环移位之类的, 没法确定

  cpu.eflags.zf = (0 == ret);            // zf
  cpu.eflags.pf = (1 == ones(ret) % 2);  // pf

  return ret;
}

word_t shr(int w, word_t op1, word_t op2) {
  assert(w == 4);
  word_t ret = op1 >> op2;

  // TODO: sf, cf, of. 可能有循环移位之类的, 没法确定

  cpu.eflags.zf = (0 == ret);            // zf
  cpu.eflags.pf = (1 == ones(ret) % 2);  // pf

  return ret;
}

word_t imul2(int w, word_t op1, word_t op2) {
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

  cpu.eflags.pf = (1 == ones(ret) % 2);  // pf
  cpu.eflags.zf = (0 == ret);            // zf
  cpu.eflags.sf = (ret & sign_mask);     // sf

  // af, do nothing

  return (word_t)ret;
}
