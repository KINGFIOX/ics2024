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

#include "local-include/reg.h"

typedef union {
  struct {
    uint8_t R_M : 3;
    uint8_t reg : 3;
    uint8_t mod : 2;
  };
  struct {
    uint8_t dont_care : 3;
    uint8_t opcode : 3;
  };
  uint8_t val;
} ModR_M;

typedef union {
  struct {
    uint8_t base : 3;
    uint8_t index : 3;
    uint8_t ss : 2;
  };
  uint8_t val;
} SIB;

static word_t x86_inst_fetch(Decode *s, int len) {
#if defined(CONFIG_ITRACE) || defined(CONFIG_IQUEUE)
  uint8_t *p = &s->isa.inst[s->snpc - s->pc];
  word_t ret = inst_fetch(&s->snpc, len);
  word_t ret_save = ret;
  int i;
  assert(s->snpc - s->pc < sizeof(s->isa.inst));
  for (i = 0; i < len; i++) {
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

static void reg_write(int idx, int width, word_t data) {
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

static void load_addr(Decode *s, ModR_M *m, word_t *rm_addr) {
  assert(m->mod != 3);

  sword_t disp = 0;
  int disp_size = 4;
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

  if (m->mod == 0) {
    if (base_reg == R_EBP) {
      base_reg = -1;
    } else {
      disp_size = 0;
    }
  } else if (m->mod == 1) {
    disp_size = 1;
  }

  if (disp_size != 0) { /* has disp */
    disp = x86_inst_fetch(s, disp_size);
    if (disp_size == 1) {
      disp = (int8_t)disp;
    }
  }

  word_t addr = disp;
  if (base_reg != -1) addr += reg_l(base_reg);
  if (index_reg != -1) addr += reg_l(index_reg) << scale;
  *rm_addr = addr;
}

static void decode_rm(Decode *s, int *rm_reg, word_t *rm_addr, int *reg, int width) {
  ModR_M m;
  m.val = x86_inst_fetch(s, 1);
  if (reg != NULL) *reg = m.reg;
  if (m.mod == 3)
    *rm_reg = m.R_M;
  else {
    load_addr(s, &m, rm_addr);
    *rm_reg = -1;
  }
}

#define Rr reg_read
#define Rw reg_write
#define Mr vaddr_read
#define Mw vaddr_write
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
  TYPE_I2r,  // XX <- Ib / eXX <- Iv
  TYPE_I2a,  // AL <- Ib / eAX <- Iv
  TYPE_G2E,  // Eb <- Gb / Ev <- Gv, General
  TYPE_E2G,  // Gb <- Eb / Gv <- Ev
  TYPE_I2E,  // Eb <- Ib / Ev <- Iv, Either
  TYPE_Ib2E,
  TYPE_cl2E,
  TYPE_1_E,
  TYPE_SI2E,
  TYPE_Eb2G,
  TYPE_Ew2G,
  TYPE_O2a,
  TYPE_a2O,
  TYPE_I_E2G,   // Gv <- EvIb / Gv <- EvIv // use for imul
  TYPE_SI_E2G,  // Gv <- EvIb / Gv <- EvIv // use for imul
  TYPE_Ib_G2E,  // Ev <- GvIb // use for shld/shrd
  TYPE_cl_G2E,  // Ev <- GvCL // use for shld/shrd
  TYPE_N,       // none
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

static void decode_operand(Decode *s, uint8_t opcode, int *rd_, word_t *src1, word_t *addr, int *rs, int *gp_idx, word_t *imm, int w, int type) {
  switch (type) {
    case TYPE_I2r:
      destr(opcode & 0x7);
      imm();
      break;
    case TYPE_G2E:
      decode_rm(s, rd_, addr, rs, w);
      src1r(*rs);
      break;
    case TYPE_E2G:
      decode_rm(s, rs, addr, rd_, w);
      break;
    case TYPE_I2E:
      decode_rm(s, rd_, addr, gp_idx, w);
      imm();
      break;
    case TYPE_O2a:
      destr(R_EAX);
      *addr = x86_inst_fetch(s, 4);
      break;
    case TYPE_a2O:
      *rs = R_EAX;
      *addr = x86_inst_fetch(s, 4);
      break;
    case TYPE_N:
      break;
    default:
      panic("Unsupported type = %d", type);
  }
}

// gp1's gp_idx from INSTPAT_START
#define gp1()         \
  do {                \
    switch (gp_idx) { \
      default:        \
        INV(s->pc);   \
    };                \
  } while (0)

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
  INSTPAT("???? ????", inv, N, 0, INV(s->pc));
  INSTPAT_END();
}

int isa_exec_once(Decode *s) {
  bool is_operand_size_16 = false;
  uint8_t opcode = 0;

again:
  opcode = x86_inst_fetch(s, 1);

  INSTPAT_START();

  // INSTPAT( pattern, name, type, width, BLOCK )

  INSTPAT("0000 1111", 2byte_esc, N, 0, _2byte_esc(s, is_operand_size_16));

  INSTPAT("0110 0110", data_size, N, 0, is_operand_size_16 = true; goto again;);

  // A0       MOV AL,moffs8
  INSTPAT("1000 0000", gp1, I2E, 1, gp1());

  // 88  /r   MOV r/m8,r8
  INSTPAT("1000 1000", mov, G2E, 1, RMw(src1));  // register memory write
  // 89  /r   MOV r/m16,r16
  // 89  /r   MOV r/m32,r32
  INSTPAT("1000 1001", mov, G2E, 0, RMw(src1));

  // 8A  /r   MOV r8,r/m8
  INSTPAT("1000 1010", mov, E2G, 1, Rw(rd, w, RMr(rs, w)));
  // 8B  /r   MOV r16,r/m16
  // 8B  /r   MOV r32,r/m32
  INSTPAT("1000 1011", mov, E2G, 0, Rw(rd, w, RMr(rs, w)));

  // A0       MOV AL,moffs8     4
  INSTPAT("1010 0000", mov, O2a, 1, Rw(R_EAX, 1, Mr(addr, 1)));
  // A1       MOV AX,moffs16    4
  INSTPAT("1010 0001", mov, O2a, 0, Rw(R_EAX, w, Mr(addr, w)));

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

  INSTPAT("1100 1100", nemu_trap, N, 0, NEMUTRAP(s->pc, cpu.eax));

  INSTPAT("???? ????", inv, N, 0, INV(s->pc));

  INSTPAT_END();
  // __instpat_end_:;

  return 0;
}
