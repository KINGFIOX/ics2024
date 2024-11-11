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

#include "../reg.h"

#include <isa.h>
#include <stdio.h>

void reg_test() {
  // emulate stat before reset
  word_t sample[8];

  word_t pc_sample = rand();
  cpu.pc = pc_sample;

  word_t eflags_sample = rand();
  cpu._val_eflags = eflags_sample;

  int i;
  for (i = R_EAX; i <= R_EDI; i++) {
    sample[i] = rand();
    reg_l(i) = sample[i];
    assert(reg_w(i) == (sample[i] & 0xffff));
  }

  assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
  assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
  assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
  assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
  assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
  assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
  assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
  assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

  assert(sample[R_EAX] == cpu.eax);
  assert(sample[R_ECX] == cpu.ecx);
  assert(sample[R_EDX] == cpu.edx);
  assert(sample[R_EBX] == cpu.ebx);
  assert(sample[R_ESP] == cpu.esp);
  assert(sample[R_EBP] == cpu.ebp);
  assert(sample[R_ESI] == cpu.esi);
  assert(sample[R_EDI] == cpu.edi);

  assert(pc_sample == cpu.pc);
  assert(eflags_sample == cpu._val_eflags);
}

void isa_reg_display() {
  for (int i = R_EAX; i <= R_EDI; i++) {
    printf("%s = 0x%08x\n", reg_name(i, 4), reg_l(i));
  }
  printf("eip = 0x%08x\n", cpu.pc);
  printf("eflags = 0x%08x\n", cpu._val_eflags);  // TODO: 可能要显示详细参数吧
}

word_t isa_reg_str2val(const char *s, bool *success) {
  *success = true;
  if (0 == strcmp(s, "eflags")) {
    return cpu._val_eflags;
  } else if (0 == strcmp(s, "eip")) {
    return cpu.pc;
  } else if (0 == strcmp(s, "eax")) {
    return cpu.gpr[R_EAX]._32;
  } else if (0 == strcmp(s, "ecx")) {
    return cpu.gpr[R_ECX]._32;
  } else if (0 == strcmp(s, "edx")) {
    return cpu.gpr[R_EDX]._32;
  } else if (0 == strcmp(s, "ebx")) {
    return cpu.gpr[R_EBX]._32;
  } else if (0 == strcmp(s, "esp")) {
    return cpu.gpr[R_ESP]._32;
  } else if (0 == strcmp(s, "ebp")) {
    return cpu.gpr[R_EBP]._32;
  } else if (0 == strcmp(s, "esi")) {
    return cpu.gpr[R_ESI]._32;
  } else if (0 == strcmp(s, "edi")) {
    return cpu.gpr[R_EDI]._32;
  } else if (0 == strcmp(s, "ax")) {
    return cpu.gpr[R_EAX]._16;
  } else if (0 == strcmp(s, "cx")) {
    return cpu.gpr[R_ECX]._16;
  } else if (0 == strcmp(s, "dx")) {
    return cpu.gpr[R_EDX]._16;
  } else if (0 == strcmp(s, "bx")) {
    return cpu.gpr[R_EBX]._16;
  } else if (0 == strcmp(s, "sp")) {
    return cpu.gpr[R_ESP]._16;
  } else if (0 == strcmp(s, "bp")) {
    return cpu.gpr[R_EBP]._16;
  } else if (0 == strcmp(s, "si")) {
    return cpu.gpr[R_ESI]._16;
  } else if (0 == strcmp(s, "di")) {
    return cpu.gpr[R_EDI]._16;
  } else if (0 == strcmp(s, "al")) {
    return cpu.gpr[R_EAX]._8[0];
  } else if (0 == strcmp(s, "ah")) {
    return cpu.gpr[R_EAX]._8[1];
  } else if (0 == strcmp(s, "bl")) {
    return cpu.gpr[R_EBX]._8[0];
  } else if (0 == strcmp(s, "bh")) {
    return cpu.gpr[R_EBX]._8[1];
  } else if (0 == strcmp(s, "cl")) {
    return cpu.gpr[R_ECX]._8[0];
  } else if (0 == strcmp(s, "ch")) {
    return cpu.gpr[R_ECX]._8[1];
  } else if (0 == strcmp(s, "dl")) {
    return cpu.gpr[R_EDX]._8[0];
  } else if (0 == strcmp(s, "dh")) {
    return cpu.gpr[R_EDX]._8[1];
  } else {
    *success = false;
    return 0;
  }
}
