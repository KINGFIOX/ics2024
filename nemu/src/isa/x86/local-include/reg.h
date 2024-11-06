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

#ifndef __X86_REG_H__
#define __X86_REG_H__

#include <isa.h>

enum { PRIV_IRET };

static inline int check_reg_index(int index) {
  IFDEF(CONFIG_RT_CHECK, assert(index >= 0 && index < 8));
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)                   // long
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)                   // word
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])  // byte

static inline const char* reg_name(int index, int width) {
  static const char* regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
  static const char* regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
  static const char* regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

  IFDEF(CONFIG_RT_CHECK, assert(index >= 0 && index < 8));

  switch (width) {
    case 4:
      return regsl[index];
    case 2:
      return regsw[index];
    case 1:
      return regsb[index];
    default:
      assert(0);
  }
}

/// @brief segment register
static inline const char* sreg_name(int index) {
  static const char* name[] = {"es", "cs", "ss", "ds", "fs", "gs"};

  IFDEF(CONFIG_RT_CHECK, assert(index >= 0 && index < ARRLEN(name)));

  return name[index];
}

#endif
