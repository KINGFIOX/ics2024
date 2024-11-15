/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>
#include <stdint.h>
#include <stdio.h>

#include "../reg.h"
#include "common.h"
#include "inst.h"
#include "isa-def.h"
#include "isa.h"
#include "macro.h"

// +-----------+-----------+-----------+--------+------+------+------+------------+-----------+
// |instruction| address-  |  operand- |segment |opcode|ModR/M| SIB  |displacement| immediate |
// |  prefix   |size prefix|size prefix|override|      |      |      |            |           |
// |-----------+-----------+-----------+--------+------+------+------+------------+-----------|
// |   0 OR 1  |  0 OR 1   |   0 OR 1  | 0 OR 1 |1 OR 2|0 OR 1|0 OR 1| 0,1,2 OR 4 |0,1,2 OR 4 |
// | - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|
// |                                     number of bytes                                      |
// +------------------------------------------------------------------------------------------+
//
//  66 c7 84 99 00 e0 ff ff 01 00      movw   $0x1,-0x2000(%ecx,%ebx,4)
//
// +-----------+-----------+-----------+--------+------+------+------+------------+-----------+
// |instruction| address-  |  operand- |segment |opcode|ModR/M| SIB  |displacement| immediate |
// |  prefix   |size prefix|size prefix|override|      |      |      |            |           |
// |-----------+-----------+-----------+--------+------+------+------+------------+-----------|
// |                            66                 c7     84     99    00 e0 ff ff    01 00   |
// +------------------------------------------------------------------------------------------+

/// @brief register or memory
typedef union {
  struct {
    uint8_t R_M : 3;
    uint8_t reg : 3;
    uint8_t mod : 2;
  };
  struct {
    uint8_t : 3;
    uint8_t opcode : 3;
  };
  uint8_t val;
} ModR_M;

/// scale index base
typedef union {
  struct {
    uint8_t base : 3;
    uint8_t index : 3;
    uint8_t ss : 2;  // scale
  };
  uint8_t val;
} SIB;

// s->snpc = 0x00100000, s->pc = 0x00100000, len = 1
// s->snpc = 0x00100001, s->pc = 0x00100000, len = 4
// 0x00100000: b8 (34 12) 00 00          movl        $0x1234, %eax
//
// s->snpc = 0x00100005, s->pc = 0x00100005, len = 1
// s->snpc = 0x00100006, s->pc = 0x00100005, len = 4
// 0x00100005: b9 (27 00 10) 00          movl        $0x100027, %ecx
//
// s->snpc = 0x0010000a, s->pc = 0x0010000a, len = 1
// s->snpc = 0x0010000b, s->pc = 0x0010000a, len = 1
// 0x0010000a: 89 01                   movl        %eax, (%ecx)
//
// s->snpc = 0x0010000c, s->pc = 0x0010000c, len = 1
// s->snpc = 0x0010000d, s->pc = 0x0010000c, len = 1
// s->snpc = 0x0010000e, s->pc = 0x0010000c, len = 1
// s->snpc = 0x0010000f, s->pc = 0x0010000c, len = 1
// s->snpc = 0x00100010, s->pc = 0x0010000c, len = 2
// 0x0010000c: 66 c7 41 04 01 00       movw        $1, 4(%ecx)
//
// s->snpc = 0x00100012, s->pc = 0x00100012, len = 1
// s->snpc = 0x00100013, s->pc = 0x00100012, len = 4
// 0x00100012: bb (02 00 00 00)          movl        $2, %ebx
//
// s->snpc = 0x00100017, s->pc = 0x00100017, len = 1
// s->snpc = 0x00100018, s->pc = 0x00100017, len = 1
// s->snpc = 0x00100019, s->pc = 0x00100017, len = 1
// s->snpc = 0x0010001a, s->pc = 0x00100017, len = 1
// s->snpc = 0x0010001b, s->pc = 0x00100017, len = 4
// s->snpc = 0x0010001f, s->pc = 0x00100017, len = 2
// 0x00100017: 66 c7 84 99 (00 e0 ff ff) 01 00 movw  $1, -0x2000(%ecx, %ebx, 4)
//
// s->snpc = 0x00100021, s->pc = 0x00100021, len = 1
// s->snpc = 0x00100022, s->pc = 0x00100021, len = 4
// 0x00100021: b8 (00 00 00 00)          movl        $0, %eax
//
// s->snpc = 0x00100026, s->pc = 0x00100026, len = 1
// 0x00100026: cc                      int3

/**
 * @brief
 *
 * @param s (mut)
 * - s->snpc (mut)
 * - s->isa.inst (mut if ITRACE)
 * @param len (input)
 * @return word_t (load from dram)
 */
static word_t x86_inst_fetch(Decode *s, int len) {
#if defined(CONFIG_ITRACE) || defined(CONFIG_IQUEUE)
  uint8_t *p = &s->isa.inst[s->snpc - s->pc];  // uint8_t inst[16];
  // printf("s->snpc = 0x%08x, s->pc = 0x%08x, len = %d\n", s->snpc, s->pc, len);
  word_t ret = inst_fetch(&s->snpc, len);
  word_t ret_save = ret;
  assert(s->snpc - s->pc < sizeof(s->isa.inst));
  int i;
  for (i = 0; i < len; i++) {  // save the inst to s->isa.inst
    p[i] = ret & 0xff;
    ret >>= 8;
  }
  return ret_save;
#else
  return inst_fetch(&s->snpc, len);
#endif
}

word_t reg_read(int idx, int width) {
  switch (width) {
    case 4:
      return reg_l(idx);
    case 1:
      return reg_b(idx);
    case 2:
      return reg_w(idx);
    default:
      assert(0);
  }
}

void reg_write(int idx, int width, word_t data) {
  switch (width) {
    case 4:
      reg_l(idx) = data;
      return;
    case 1:
      reg_b(idx) = data;
      return;
    case 2:
      reg_w(idx) = data;
      return;
    default:
      assert(0);
  }
}

/**
 * @brief
 *
 * @param s (mut)
 * @param m
 * @param rm_addr (return)
 */
static void load_addr(Decode *s, const ModR_M *m, word_t *rm_addr) {
  assert(m->mod != 3);

  sword_t disp = 0;
  int disp_size = 4;  // displacement size
  int base_reg = -1, index_reg = -1, scale = 0;

  if (m->R_M == R_ESP) {
    SIB sib;
    sib.val = x86_inst_fetch(s, 1);
    base_reg = sib.base;
    scale = sib.ss;

    if (sib.index != R_ESP) {
      index_reg = sib.index;
    }
  } else {
    base_reg = m->R_M;
  } /* no SIB */

  if (0 == m->mod) {
    if (base_reg == R_EBP) {
      base_reg = -1;
    } else {
      disp_size = 0;
    }
  } else if (1 == m->mod) {
    disp_size = 1;
  }

  if (disp_size != 0) { /* has disp */
    disp = x86_inst_fetch(s, disp_size);
    if (disp_size == 1) {
      disp = (int8_t)disp;
    }
  }

  // addr = displace + index * scala + base
  word_t addr = disp;
  if (base_reg != -1) addr += reg_l(base_reg);
  if (index_reg != -1) addr += reg_l(index_reg) << scale;
  *rm_addr = addr;
}

/**
 * @brief
 *
 * @param s (mut)
 * @param rm_reg (return)
 * @param rm_addr
 * @param reg (return)
 * @param width
 */
static void decode_rm(Decode *s, int *rm_reg, word_t *rm_addr, int *reg, int width) {
  ModR_M m;
  m.val = x86_inst_fetch(s, 1);
  if (reg != NULL) *reg = m.reg;
  // i386 manual 17.2
  if (3 == m.mod) {
    *rm_reg = m.R_M;
  } else {
    load_addr(s, &m, rm_addr);
    *rm_reg = -1;
  }
}

#define RMr(reg, w) (reg != -1 ? Rr(reg, w) : Mr(addr, w))

#define RMw(data)        \
  do {                   \
    if (rd != -1)        \
      Rw(rd, w, data);   \
    else                 \
      Mw(addr, w, data); \
  } while (0)

#define destr(r) \
  do {           \
    *rd_ = (r);  \
  } while (0)

#define src1r(r)      \
  do {                \
    *src1 = Rr(r, w); \
  } while (0)

#define imm()                    \
  do {                           \
    *imm = x86_inst_fetch(s, w); \
  } while (0)

/// @brief sign extend immediate
#define simm(w)                               \
  do {                                        \
    *imm = SEXT(x86_inst_fetch(s, w), w * 8); \
  } while (0)

enum {
  TYPE_r,
  TYPE_I,
  TYPE_SI,
  TYPE_J,
  TYPE_E,
  TYPE_a2dx,
  TYPE_I2r,   // XX <- Ib / eXX <- Iv, Immediate
  TYPE_I2a,   // AL <- Ib / eAX <- Iv, a means
  TYPE_G2E,   // Eb <- Gb / Ev <- Gv, General
  TYPE_E2G,   // Gb <- Eb / Gv <- Ev
  TYPE_I2E,   // Eb <- Ib / Ev <- Iv, Either
  TYPE_Ib2E,  // E <- byte
  TYPE_cl2E,  // Either <- cl(low byte of ecx)
  TYPE_1_E,
  TYPE_SI2E,  // Either <- scala index base
  TYPE_Eb2G,
  TYPE_Ew2G,
  TYPE_O2a,     // ax <- offset content of memory
  TYPE_a2O,     // offset of memory <- ax
  TYPE_I_E2G,   // Gv <- EvIb / Gv <- EvIv // use for imul
  TYPE_SI_E2G,  // Gv <- EvIb / Gv <- EvIv // use for imul
  TYPE_Ib_G2E,  // Ev <- GvIb // use for shld/shrd
  TYPE_cl_G2E,  // Ev <- GvCL // use for shld/shrd
  TYPE_N,       // none
  TYPE_a2r,
  TYPE_dx2a,
};

#define INSTPAT_INST(s) opcode
#define INSTPAT_MATCH(s, name, type, width, ... /* execute body */)                           \
  {                                                                                           \
    int rd = 0, rs = 0, gp_idx = 0;                                                           \
    word_t src1 = 0, addr = 0, imm = 0;                                                       \
    int w = width == 0 ? (is_operand_size_16 ? 2 : 4) : width;                                \
    decode_operand(s, opcode, &rd, &src1, &addr, &rs, &gp_idx, &imm, w, concat(TYPE_, type)); \
    s->dnpc = s->snpc;                                                                        \
    __VA_ARGS__;                                                                              \
  }

static inline void idiv(int w, word_t op) {
  assert(w == 4);
  sword_t divisor = op;
  int64_t dividend = ((int64_t)Rr(R_EDX, 4) << 32) | (int64_t)Rr(R_EAX, 4);
  Rw(R_EAX, 4, dividend / divisor);
  Rw(R_EDX, 4, (word_t)(dividend % divisor));
}

/**
 * @brief
 *
 * @param s (return)
 * @param opcode (input)
 * @param rd_ (return)
 * @param src1 (return)
 * @param addr (return)
 * @param rs (return)
 * @param gp_idx (return) index for SIB
 * @param imm (return)
 * @param w (input)
 * @param type (input)
 */
static void decode_operand(Decode *s, uint8_t opcode, int *rd_, word_t *src1, word_t *addr, int *rs, int *gp_idx, word_t *imm, int w, int type) {
  switch (type) {
    case TYPE_a2r:
      decode_rm(s, rd_, addr, gp_idx, w);
      break;
    case TYPE_a2dx:
      break;
    case TYPE_dx2a:
      break;
    case TYPE_SI_E2G:
      decode_rm(s, rs, addr, rd_, w);
      simm(1);
      break;
    case TYPE_Eb2G:
      decode_rm(s, rd_, addr, rs, 1);
      break;
    case TYPE_Ew2G:
      decode_rm(s, rd_, addr, rs, 2);
      break;
    case TYPE_I2r:
      destr(opcode & 0b0111);
      imm();  // decode mean while inst fetch
      break;
    case TYPE_G2E:  // General to Either
      decode_rm(s, rd_, addr, rs, w);
      break;
    case TYPE_E2G:
      decode_rm(s, rs, addr, rd_, w);
      break;
    case TYPE_I2E:
      decode_rm(s, rd_, addr, gp_idx, w);
      imm();
      break;
    case TYPE_O2a:
      destr(R_EAX);  // *rd_ = R_EAX;
      *addr = x86_inst_fetch(s, 4);
      break;
    case TYPE_a2O:
      *rs = R_EAX;
      *addr = x86_inst_fetch(s, 4);
      break;
    case TYPE_E:
      decode_rm(s, rd_, addr, gp_idx, w);
      break;
    case TYPE_I:
      imm();
      break;
    case TYPE_J:
      // imm();
      if (4 == w) {
        // *imm = (word_t)(int32_t)*imm;
        simm(4);
      } else if (2 == w) {
        // *imm = (word_t)(int32_t)(int16_t)*imm;
        simm(2);
      } else if (1 == w) {
        // *imm = (word_t)(int32_t)(int8_t)*imm;
        simm(1);
      }
      break;
    case TYPE_SI:
      simm(1);
      break;
    case TYPE_Ib2E:
      decode_rm(s, rd_, addr, gp_idx, w);
      simm(1);
      break;
    case TYPE_cl_G2E:
      decode_rm(s, rd_, addr, rs, w);
      break;
    case TYPE_cl2E:
      decode_rm(s, rd_, addr, gp_idx, w);
      break;
    case TYPE_SI2E:
      decode_rm(s, rd_, addr, gp_idx, w);
      simm(1);
      break;
    case TYPE_1_E:
      decode_rm(s, rd_, addr, gp_idx, w);
      break;
    case TYPE_I2a:
      imm();
      break;
    case TYPE_r:
      destr(opcode & 0b0111);
      break;
    case TYPE_N:
      break;
    default:
      panic("Unsupported type = %d", type);
  }
}

// gp1's gp_idx from INSTPAT_START
#define gp1()                                     \
  do {                                            \
    switch (gp_idx) {                             \
      case 0b000: /*rd=rd+imm*/                   \
        RMw(add(w, RMr(rd, w), imm, false));      \
        break;                                    \
      case 0b001: /*rd=rd or imm*/                \
        RMw(or_(w, RMr(rd, w), imm));             \
        break;                                    \
      case 0b010: /*cmp*/                         \
        cmp(w, RMr(rd, w), imm);                  \
        break;                                    \
      case 0b100: /*rd=rd&imm*/                   \
        RMw(and_(w, RMr(rd, w), imm));            \
        break;                                    \
      case 0b101: /*rd=rd-imm*/                   \
        RMw(sub(w, RMr(rd, w), imm, false));      \
        break;                                    \
      case 0b110: /*rd=rd xor imm*/               \
        RMw(xor_(w, RMr(rd, w), imm));            \
        break;                                    \
      case 0b111: /*cmp*/                         \
        cmp(w, RMr(rd, w), imm);                  \
        break;                                    \
      default:                                    \
        Assert(false, "gp_idx = 0b%03b", gp_idx); \
    };                                            \
  } while (0)

#define gp5()                                      \
  do {                                             \
    switch (gp_idx) {                              \
      case 0b000: /*inc*/                          \
        Mw(addr, w, add(w, RMr(rd, w), 1, false)); \
        break;                                     \
      case 0b001: /*dec*/                          \
        Mw(addr, w, sub(w, RMr(rd, w), 1, false)); \
        break;                                     \
      case 0b010: /*call*/                         \
        calla(s, w, RMr(rd, w));                   \
        break;                                     \
      case 0b100: /*jmp*/                          \
        jmpa(s, RMr(rd, w));                       \
        break;                                     \
      case 0b110:                                  \
        push(w, RMr(rd, w));                       \
        break;                                     \
      default:                                     \
        Assert(false, "gp_idx = 0b%03b", gp_idx);  \
    }                                              \
  } while (0)

#define gp7()                                           \
  do {                                                  \
    switch (gp_idx) {                                   \
      case 0b011: /*lidtl*/                             \
        cpu.idtr = Rr(rs, w);                           \
        break;                                          \
      case 0b110:                                       \
        Rw(rd, w, add(w, Rr(rd, w), Rr(rs, w), false)); \
        break;                                          \
      default:                                          \
        Assert(false, "gp_idx = 0b%03b", gp_idx);       \
    }                                                   \
  } while (0)

static inline void imul1(int w, word_t op1) {
  assert(w == 4);
  if (4 == w) {
    int64_t op2_ = (int32_t)Rr(R_EAX, w);
    int64_t op1_ = (int32_t)op1;
    int64_t ret = op2_ * op1_;
    Rw(R_EAX, w, ret & UINT32_MAX);
    Rw(R_EDX, w, ret >> 32);
    cpu.eflags.sf = (ret < 0);
    cpu.eflags.zf = !ret;
    // never overflow
  } else if (2 == w) {
    assert(false);
  } else if (1 == w) {
    assert(false);
  }
}

static inline void mul1(int w, word_t op1_) {
  assert(4 == w || 2 == w || 1 == w);
  uint64_t sign_mask = (1ULL << (w * 8 - 1));
  uint64_t op2 = 0;
  uint64_t op1 = 0;
  uint64_t ret = 0;
  if (4 == w) {
    op2 = Rr(R_EAX, w);
    op1 = op1_;
    ret = op2 * op1;
    Rw(R_EAX, w, ret & UINT32_MAX);
    Rw(R_EDX, w, ret >> 32);
  } else if (2 == w) {
    op2 = Rr(R_AX, w);
    op1 = op1_ & UINT16_MAX;
    ret = op2 * op1;
    Rw(R_AX, w, ret & UINT16_MAX);
    Rw(R_DX, w, ret >> 16);
  } else if (1 == w) {
    op2 = Rr(R_AL, w);
    op1 = op1_ & UINT8_MAX;
    ret = op2 * op1;
    Rw(R_AL, w, ret & UINT8_MAX);
    Rw(R_AH, w, ret >> 8);
  }
  cpu.eflags.sf = !!(ret & sign_mask);
  cpu.eflags.zf = !ret;
}

static inline void div1(int w, word_t divisor) {
  assert(1 == w || 2 == w || 4 == w);
  if (1 == w) {
    uint64_t dividend = Rr(R_AX, 2);
    Rw(R_AL, 1, dividend / divisor);
    Rw(R_AH, 1, dividend % divisor);
  } else {
    uint64_t dividend = ((uint64_t)Rr(R_EDX, w) << (w * 8)) | Rr(R_EAX, w);
    word_t rem = dividend % divisor;
    word_t quo = dividend / divisor;
    Rw(R_EAX, w, quo);
    Rw(R_EDX, w, rem);
  }
}

#define gp3()                                                          \
  do {                                                                 \
    switch (gp_idx) {                                                  \
      case 0b000: /*test*/                                             \
        imm = x86_inst_fetch(s, w);                                    \
        s->dnpc = s->snpc;                                             \
        test(w, RMr(rd, w), imm);                                      \
        break;                                                         \
      case 0b011: /*neg*/                                              \
        RMw(sub(w, 0, RMr(rd, w), false));                             \
        break;                                                         \
      case 0b010: /*not*/                                              \
        RMw(not_(w, RMr(rd, w)));                                      \
        break;                                                         \
      case 0b100: /*test*/                                             \
        mul1(w, RMr(rd, w));                                           \
        break;                                                         \
      case 0b101: /*imul*/                                             \
        assert(w == 4);                                                \
        imul1(w, RMr(rd, w));                                          \
        break;                                                         \
      case 0b110: /*div*/                                              \
        div1(w, RMr(rd, w));                                           \
        break;                                                         \
      case 0b111: /*idiv*/                                             \
        idiv(w, RMr(rd, w));                                           \
        break;                                                         \
      default:                                                         \
        printf("%s:%d gp_idx = 0b%03b\n", __FILE__, __LINE__, gp_idx); \
        INV(s->pc);                                                    \
    }                                                                  \
  } while (0)

#define gp2()                                                          \
  do {                                                                 \
    switch (gp_idx) {                                                  \
      case 0b000:                                                      \
        RMw(rol(w, RMr(rd, w), op2));                                  \
        break;                                                         \
      case 0b001:                                                      \
        RMw(ror(w, RMr(rd, w), op2));                                  \
        break;                                                         \
      case 0b100:                                                      \
        RMw(shl(w, RMr(rd, w), op2));                                  \
        break;                                                         \
      case 0b101:                                                      \
        RMw(shr(w, RMr(rd, w), op2));                                  \
        break;                                                         \
      case 0b111:                                                      \
        RMw(sar(w, RMr(rd, w), op2));                                  \
        break;                                                         \
      default:                                                         \
        printf("%s:%d gp_idx = 0b%03b\n", __FILE__, __LINE__, gp_idx); \
        INV(s->pc);                                                    \
    }                                                                  \
  } while (0)

static inline void movz_l(int w, int rd, int rs, vaddr_t addr, bool sign, int width) {
  assert(width == 1 || width == 2 || width == 4);
  word_t op1 = RMr(rd, width);
  if (sign) {
    if (width == 1) {
      Rw(rs, w, SEXT(op1, 1 * 8));
    } else if (width == 2) {
      Rw(rs, w, SEXT(op1, 2 * 8));
    } else {
      Rw(rs, w, SEXT(op1, 4 * 8));
    }
  } else {
    Rw(rs, w, op1);
  }
}

static inline void setcc(uint8_t opcode, int rd) {
  uint8_t subcode = opcode & 0xf;
  switch (subcode) {
    case 0x5:  // setne
      Rw(rd, 1, (!cpu.eflags.zf));
      break;
    case 0x4:  // sete
      Rw(rd, 1, (cpu.eflags.zf));
      break;
    default:
      assert(false && "invalid subcode");
  }
}

static inline void shrd(int w, int rd, int rs, vaddr_t addr) {
  uint64_t op1, op2;
  op1 = Rr(rs, w);
  op2 = RMr(rd, w);
  uint64_t ret = op1 << (w * 8) | op2;
  uint64_t cl = Rr(R_CL, 1);
  ret = ret >> cl;
  RMw(ret & ((1ULL << (w * 8)) - 1));  // write rd
  Rw(rs, w, ret >> (w * 8));

  // TODO: sf, cf, of. 可能有循环移位之类的, 没法确定
  cpu.eflags.zf = (0 == ret);  // zf
  // cpu.eflags.pf = (1 == ones(ret) % 2);  // pf
}

// 0F  20 /r   MOV r32,CR0/CR2/CR3   6        Move (control register) to (register)
// 0F  22 /r   MOV CR0/CR2/CR3,r32   10/4/5   Move (register) to (control register)
// 0F  21 /r   MOV r32,DR0 -- 3      22       Move (debug register) to (register)
// 0F  21 /r   MOV r32,DR6/DR7       14       Move (debug register) to (register)
// 0F  23 /r   MOV DR0 -- 3,r32      22       Move (register) to (debug register)
// 0F  23 /r   MOV DR6/DR7,r32       16       Move (register) to (debug register)
// 0F  24 /r   MOV r32,TR6/TR7       12       Move (test register) to (register)
// 0F  26 /r   MOV TR6/TR7,r32       12       Move (register) to (test register)
void _2byte_esc(Decode *s, bool is_operand_size_16) {
  uint8_t opcode = x86_inst_fetch(s, 1);
  INSTPAT_START();
  // 0f 01 18 lidtl (%eax)
  INSTPAT("0000 0001", lidtl, E, 4, gp7());
  //   100067:       0f 94 c2                sete   %dl
  INSTPAT("1001 ????", setcc, E, 1, setcc(opcode, rd));
  // INSTPAT("1001 0???", sete, a2r, 0, Rw(rd, 1, (cpu.eflags.zf != 0)));
  //   100063:       0f af c1                imul   %ecx,%eax
  INSTPAT("1010 1111", imul2, E2G, 0, Rw(rd, w, imul2(w, Rr(rd, w), RMr(rs, w))));
  // 0f ad d0 : shrd %cl, %edx, %eax
  INSTPAT("1010 1101", shrd, cl_G2E, 0, shrd(w, rd, rs, addr));
  //   10006a:       0f b6 d2                movzbl %dl,%edx
  INSTPAT("1011 0110", movzbl, Eb2G, 0, movz_l(w, rd, rs, addr, false, 1));
  //   1000b8:       0f be 05 60 02 10 00    movsbl 0x100260,%eax
  INSTPAT("1011 1110", movsbl, Eb2G, 0, movz_l(w, rd, rs, addr, true, 1));
  //   100043:       0f bf 84 1b 40 02 10    movswl 0x100240(%ebx,%ebx,1),%eax
  INSTPAT("1011 1111", movswl, Ew2G, 4, movz_l(w, rd, rs, addr, true, 2));
  //   10006f:       0f b7 84 1b 40 02 10    movzwl 0x100240(%ebx,%ebx,1),%eax
  INSTPAT("1011 0111", movzwl, Ew2G, 4, movz_l(w, rd, rs, addr, false, 2));
  //   100160:       0f 85 8e 01 00 00       jne    1002f4 <__udivmoddi4+0x1c0>
  INSTPAT("1000 ????", jcc, J, 4, jcc(s, imm, opcode & mask));
  INSTPAT("???? ????", inv, N, 0, INV(s->pc));
  INSTPAT_END();
}

static inline void cltd() {
  int w = 4;
  int sign_mask = (1 << (w * 8 - 1));
  word_t eax = Rr(R_EAX, w);
  bool sign = (eax & sign_mask) != 0;
  if (sign) {
    Rw(R_EDX, w, all);
  } else {
    Rw(R_EDX, w, 0);
  }
}

static inline void out_(int w, word_t data, word_t port) {
  extern void pio_write(ioaddr_t addr, int len, uint32_t data);
  pio_write(port, w, data);
}

static inline word_t in_(int w, word_t port) {
  // printf("in_: addr = %x\n", port);
  uint32_t pio_read(ioaddr_t addr, int len);
  printf("0x%08x\n", port);
  return pio_read(port, w);
}

static inline void rep(Decode *s, int w) {
  uint8_t opcode = x86_inst_fetch(s, 1);
  int rs = opcode & 0b0111;
  switch (opcode) {
    case 0xa5:
      Mw(Rr(R_EDI, w), w, Mr(Rr(R_ESI, w), w));
      Rw(R_ESI, w, Rr(R_ESI, w) + w);
      Rw(R_EDI, w, Rr(R_EDI, w) + w);
      break;
    case 0xab:
      Mw(Rr(R_EDI, w), w, Rr(rs, w));
      Rw(R_EDI, w, Rr(R_EDI, w) + w);
      break;
    default:
      printf("%s:%d rep opcode = 0x%02x\n", __FILE__, __LINE__, opcode);
      INV(s->pc);
      break;
  }
  Rw(R_ECX, w, Rr(R_ECX, w) - 1);
  if (Rr(R_ECX, w) > 0) {
    s->dnpc = s->snpc - 2;
  } else {
    s->dnpc = s->snpc;
  }
}

int isa_exec_once(Decode *s) {
  bool is_operand_size_16 = false;
  uint8_t opcode = 0;

again:
  opcode = x86_inst_fetch(s, 1);

  INSTPAT_START();

  /* INSTPAT( pattern, name, type, width, BLOCK ) */

  // IMPORTANT: 66(prefix)
  INSTPAT("0110 0110", data_size, N, 0, is_operand_size_16 = true; goto again;);

  INSTPAT("0000 1111", 2byte_esc, N, 0, _2byte_esc(s, is_operand_size_16));

  // ed in (%dx),%eax
  INSTPAT("1110 1101", in, dx2a, 0, Rw(R_EAX, w, in_(w, Rr(R_DX, 2))));
  //   10016f:       ee                      out    %al,(%dx)
  INSTPAT("1110 1110", out, a2dx, 1, out_(w, Rr(R_AL, w), Rr(R_DX, 2)));
  // ef out %eax,(%dx)
  INSTPAT("1110 1111", out, a2dx, 0, out_(w, Rr(R_EAX, w), Rr(R_DX, 2)));
  //   1002d6:       e9 fc fe ff ff          jmp    1001d7 <vsnprintf+0x33>
  INSTPAT("1110 1001", jmp, J, 4, jmpo(s, imm));

  //   100010:       31 c0                   xor    %eax,%eax
  INSTPAT("0011 0001", xor, G2E, 0, Rw(rd, w, xor_(w, Rr(rd, w), Rr(rs, w))));
  //   100074:       09 d0                   or     %edx,%eax
  INSTPAT("0000 1001", or, G2E, 0, Rw(rd, w, or_(w, Rr(rd, w), Rr(rs, w))));
  //   10007c:       0a 02                   or     (%edx),%al
  INSTPAT("0000 1010", or, E2G, 1, Rw(rd, w, or_(w, Rr(rd, w), Mr(addr, w))));
  //   100071:       22 02                   and    (%edx),%al
  INSTPAT("0010 0010", and, E2G, 1, Rw(rd, w, and_(w, Rr(rd, w), Mr(addr, w))));
  //   1045f0:       21 d0                   and    %edx,%eax
  INSTPAT("0010 0001", and, G2E, 0, Rw(rd, w, and_(w, Rr(rd, w), Rr(rs, w))));
  //   10004d:       32 06                   xor    (%esi),%al
  INSTPAT("0011 0010", xor, E2G, 1, Rw(rd, w, xor_(w, Rr(rd, w), RMr(rs, w))));
  //   100052:       33 14 85 e0 01 10 00    xor    0x1001e0(,%eax,4),%edx
  INSTPAT("0011 0011", xor, E2G, 0, Rw(rd, w, xor_(w, Rr(rd, w), RMr(rs, w))));

  // A0       MOV AL,moffs8
  INSTPAT("1000 0000", gp1, I2E, 1, gp1());
  //   100070:       81 fb 00 01 00 00       cmp    $0x100,%ebx
  INSTPAT("1000 0001", gp1, I2E, 0, gp1());
  //   100017:       83 ec 14                sub    $0x14,%esp
  //   10002c:       83 e4 f0                and    $0xfffffff0,%esp
  //   1000ab:       83 f0 01                xor    $0x1,%eax
  // 83 c4 10 add $0x10,%esp
  INSTPAT("1000 0011", gp1, SI2E, 0, gp1());

  // 1004b3:       29 c8                   sub    %ecx,%eax
  INSTPAT("0010 1001", sub, G2E, 0, RMw(sub(w, RMr(rd, w), Rr(rs, w), false)));

  // 2d a8 24 29 00 sub $0x2924a8,%eax
  INSTPAT("0010 1101", sub, I2a, 0, Rw(R_EAX, w, sub(w, Rr(R_EAX, w), imm, false)));

  // 10005f:       2b 04 cd 80 03 10 00    sub    0x100380(,%ecx,8),%eax
  INSTPAT("0010 1011", sub, E2G, 0, Rw(rd, w, sub(w, Rr(rd, w), Mr(addr, w), false)));

  // 100066:       1b 14 cd 84 03 10 00    sbb    0x100384(,%ecx,8),%edx
  INSTPAT("0001 1011", sbb, E2G, 0, Rw(rd, w, sub(w, Rr(rd, w), Mr(addr, w), true)));

  // 100040:       84 04 11                test   %al,(%ecx,%edx,1)
  //   100298:       84 c0                   test   %al,%al
  INSTPAT("1000 0100", test, G2E, 1, test(w, Rr(rs, w), RMr(rd, w)));

  // 88  /r   MOV r/m8,r8
  INSTPAT("1000 1000", mov, G2E, 1, RMw(Rr(rs, w)));  // register memory write
  // 89  /r   MOV r/m16,r16
  // 89  /r   MOV r/m32,r32
  INSTPAT("1000 1001", mov, G2E, 0, RMw(Rr(rs, w)));

  // 8A  /r   MOV r8,r/m8
  INSTPAT("1000 1010", mov, E2G, 1, Rw(rd, w, RMr(rs, w)));
  // 8B  /r   MOV r16,r/m16
  // 8B  /r   MOV r32,r/m32
  INSTPAT("1000 1011", mov, E2G, 0, Rw(rd, w, RMr(rs, w)));

  // A0       MOV AL,moffs8     4
  INSTPAT("1010 0000", mov, O2a, 1, Rw(R_EAX, 1, Mr(addr, 1)));
  // A1       MOV AX,moffs16    4
  INSTPAT("1010 0001", mov, O2a, 0, Rw(R_EAX, w, Mr(addr, w)));

  //   1004b5:       19 da                   sbb    %ebx,%edx
  INSTPAT("0001 1001", sbb, G2E, 0, Rw(rd, w, sub(w, Rr(rd, w), Rr(rs, w), true)));

  //   100066:       13 14 cd 84 03 10 00    adc    0x100384(,%ecx,8),%edx
  INSTPAT("0001 0011", adc, E2G, 0, Rw(rd, w, add(w, Rr(rd, w), Mr(addr, w), true)));
  //   10005f:       11 da                   adc    %ebx,%edx
  INSTPAT("0001 0001", adc, G2E, 0, Rw(rd, w, add(w, Rr(rd, w), Rr(rs, w), true)));

  // A2       MOV moffs8,AL     2
  INSTPAT("1010 0010", mov, a2O, 1, Mw(addr, 1, Rr(R_EAX, 1)));
  // A3       MOV moffs16,AX    2
  // A3       MOV moffs32,EAX   2
  INSTPAT("1010 0011", mov, a2O, 0, Mw(addr, w, Rr(R_EAX, w)));

  // B0 + rb ib  MOV reg8,imm8 , ??? here is the id of reg
  INSTPAT("1011 0???", mov, I2r, 1, Rw(rd, 1, imm));

  // B8 + rw iw  MOV reg16,imm16
  // B8 + rd id  MOV reg32,imm32
  INSTPAT("1011 1???", mov, I2r, 0, Rw(rd, w, imm));

  // C6 ib    MOV r/m8,imm8
  INSTPAT("1100 0110", mov, I2E, 1, RMw(imm));
  // C7 iw    MOV r/m16,imm16
  // C7 id    MOV r/m32,imm32
  INSTPAT("1100 0111", mov, I2E, 0, RMw(imm));

  INSTPAT("0111 ????", jcc, J, 1, jcc(s, imm, opcode & mask));

  INSTPAT("1100 1001", leave, N, 0, leave());

  //   100012:       c3                      ret
  INSTPAT("1100 0011", ret, N, 0, ret(s, w));

  //   10000a:       e8 05 00 00 00          call   100014 <_trm_init>
  INSTPAT("1110 1000", call, J, 4, callo(s, w, imm));

  //   100028:       8d 4c 24 04             lea    0x4(%esp),%ecx
  INSTPAT("1000 1101", lea, E2G, 0, Rw(rd, w, addr));

  //   100014:       55                      push   %ebp
  INSTPAT("0101 0???", pushl, r, 0, push(w, Rr(rd, w)));
  //   10001a:       68 40 00 10 00          push   $0x100040
  INSTPAT("0110 1000", push, I, 0, push(w, imm));
  //
  INSTPAT("0110 1010", push, SI, 0, push(w, imm));  // 这个 width = 0 是试出来的

  INSTPAT("0110 1011", imul3, SI_E2G, 0, Rw(rd, w, imul2(w, RMr(rs, w), imm)));

  INSTPAT("0101 1???", pop, r, 0, Rw(rd, w, pop(w)));

  //   100060:       3b 94 bb 60 01 10 00    cmp    0x100160(%ebx,%edi,4),%edx
  INSTPAT("0011 1011", cmp, E2G, 0, cmp(w, Rr(rd, w), Mr(addr, w)));
  //   1000d3:       3d 39 a3 4f 41          cmp    $0x414fa339,%eax
  INSTPAT("0011 1101", cmp, I2a, 0, cmp(w, Rr(R_EAX, w), imm));
  //   10003f:       3c 19                   cmp    $0x19,%al
  INSTPAT("0011 1100", cmp, I2a, 1, cmp(w, Rr(R_EAX, w), imm));
  //   10005c:       39 f1                   cmp    %esi,%ecx
  //   100054:       39 04 9d 40 01 10 00    cmp    %eax,0x100140(,%ebx,4)
  INSTPAT("0011 1001", cmp, E2G, 0, cmp(w, RMr(rs, w), Rr(rd, w)));
  //   10004c:       38 d8                   cmp    %bl,%al
  INSTPAT("0011 1000", cmp, G2E, 1, cmp(w, RMr(rd, w), Rr(rs, w)));

  INSTPAT("0100 0???", inc, r, 0, Rw(rd, w, add(w, Rr(rd, w), 1, false)));

  //   100090:       49                      dec    %ecx
  INSTPAT("0100 1???", dec, r, 0, Rw(rd, w, sub(w, Rr(rd, w), 1, false)));

  // 10005e:       01 f2                   add    %esi,%edx
  INSTPAT("0000 0001", add, G2E, 0, RMw(add(w, RMr(rd, w), Rr(rs, w), false)));

  //   100043:       03 04 9d dc 01 10 00    add    0x1001dc(,%ebx,4),%eax
  INSTPAT("0000 0011", add, E2G, 0, Rw(rd, w, add(w, Rr(rd, w), Mr(addr, w), false)));

  // 05 62 07 00 00 : add $0x762, %eax
  INSTPAT("0000 0101", add, I2a, 0, Rw(rd, w, add(w, Rr(rd, w), imm, false)));

  //   100036:       85 db                   test   %ebx,%ebx
  INSTPAT("1000 0101", test, G2E, 0, test(w, Rr(rd, w), Rr(rs, w)));

  //   10007f:       90                      nop
  INSTPAT("1001 0000", nop, N, 0, /*nop*/);

  //   10002f:       ff 71 fc                push   -0x4(%ecx)
  INSTPAT("1111 1111", gp5, E, 0, gp5());
  //   10004c:       f6 c3 03                test   $0x3,%bl
  INSTPAT("1111 0110", gp3, E, 1, gp3());
  //   100085:       f7 d8                   neg    %eax
  INSTPAT("1111 0111", gp3, E, 0, gp3());
  //   100799:       f3 a5                   rep movsl %ds:(%esi),%es:(%edi)
  INSTPAT("1111 0011", rep, N, 0, rep(s, w));

  //   100087:       25 20 83 b8 ed          and    $0xedb88320,%eax
  INSTPAT("0010 0101", and, I2a, 0, Rw(R_EAX, w, and_(w, Rr(R_EAX, w), imm)));

  //   10008a:       99                      cltd
  INSTPAT("1001 1001", cltd, N, 0, cltd());
  INSTPAT("1001 1000", cwtl, N, 0, Rw(R_EAX, 4, SEXT(Rr(R_AX, 2), 2 * 8)));

  //   10008c:       d1 ea                   shr    $1,%edx
  INSTPAT("1101 0001", shr, 1_E, 0, word_t op2 = 1; gp2());
  // d0 f8 sar $1,%al
  INSTPAT("1101 0000", sar, 1_E, 1, word_t op2 = 1; gp2());
  //   10004a:       c1 ea 08                shr    $0x8,%edx
  INSTPAT("1100 0001", shr, Ib2E, 0, word_t op2 = imm; gp2());
  // c0 f8 02 sar $0xw,%al
  INSTPAT("1100 0000", gp2, Ib2E, 1, word_t op2 = imm; gp2());

  //   1000f0:       d3 e0                   shl    %cl,%eax
  INSTPAT("1101 0011", gp2, cl2E, 0, word_t op2 = Rr(R_CL, 1); gp2());

  // cd 81 int $0x81
  INSTPAT("1100 1101", int_, I, 1, int_(s, imm, cpu.pc));

  INSTPAT("1100 1100", nemu_trap, N, 0, NEMUTRAP(s->pc, cpu.eax));

  //   100093:       eb d3                   jmp    100068 <rc_crc32+0x40>
  INSTPAT("1110 1011", jmp, J, 1, jmpo(s, imm));

  INSTPAT("???? ????", inv, N, 0, INV(s->pc));

  INSTPAT_END();
  // __instpat_end_:;

  return 0;
}
