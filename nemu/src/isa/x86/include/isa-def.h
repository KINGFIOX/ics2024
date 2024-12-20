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

#ifndef __ISA_X86_H__
#define __ISA_X86_H__

#include <common.h>
#include <stdint.h>

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

// 这是一个 union 数组
// 然后这个结构体刚好与 union 的定义一一对应, 当然 pc 是多出来的

typedef union {
  union GPR {
    uint32_t _32;
    uint16_t _16;
    uint8_t _8[2];
  } gpr[8];  // general purpose register

  /* Do NOT change the order of the GPRs' definitions. */
  struct {
    uint32_t eax /*gpr[0]*/, ecx /*gpr[1]*/, edx /*gpr[2]*/, ebx /*gpr[3]*/, esp /*gpr[4]*/, ebp /*gpr[5]*/, esi /*gpr[6]*/, edi /*gpr[7]*/;
    vaddr_t pc;
    union {
      struct {
        uint32_t cf : 1;
        uint32_t : 1;  // always 1 in eflags
        uint32_t pf : 1;
        uint32_t : 1;
        uint32_t : 1;
        uint32_t : 1;
        uint32_t zf : 1;
        uint32_t sf : 1;

        uint32_t tf : 1;
        uint32_t if_ : 1;
        uint32_t df : 1;
        uint32_t of : 1;
        uint32_t iopl : 2;
        uint32_t nt : 1;
        uint32_t md : 1;

        uint32_t rf : 1;
        uint32_t vm : 1;
        uint32_t ac : 1;
        uint32_t vif : 1;
        uint32_t vip : 1;
        uint32_t id : 1;
        uint32_t : 8;
        uint32_t : 1;
        uint32_t ai : 1;
      } eflags;
      uint32_t _val_eflags;
    };
    uint32_t idtr;  // 中断描述符表寄存器

    bool INTR;
  };

} x86_CPU_state;

// decode
typedef struct {
  uint8_t inst[16];
  uint8_t *p_inst;
} x86_ISADecodeInfo;

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)
#endif
