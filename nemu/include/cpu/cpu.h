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

#ifndef __CPU_CPU_H__
#define __CPU_CPU_H__

#include <common.h>
#include <stdint.h>
#include <stdlib.h>

void cpu_exec(uint64_t n);

void set_nemu_state(int state, vaddr_t pc, int halt_ret);
void invalid_inst(vaddr_t thispc);

#define NEMUTRAP(thispc, code) set_nemu_state(NEMU_END, thispc, code)
#define INV(thispc) invalid_inst(thispc)

#ifdef CONFIG_ITRACE
#define ITRACE_BUF_SIZE 32
typedef struct {
  vaddr_t pc;
  int len;
} iringbuf_t;

extern iringbuf_t iringbuf[];
extern int rear;

static inline void print_iringbuf() {
  for (int i = (rear + 1) % ITRACE_BUF_SIZE; i != rear; i = (i + 1) % ITRACE_BUF_SIZE) {
    if (iringbuf[i].len != 0) {
      // disassemble(p, s->logbuf + sizeof(s->logbuf) - p, MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst, ilen);
      // disassemble(iringbuf[i].str, sizeof(iringbuf[i].str), iringbuf[i].pc, (uint8_t *)&iringbuf[i].code, iringbuf[i].len);
      uint8_t *code = malloc(iringbuf[i].len);
      memset(code, 0, iringbuf[i].len);
      char str[128];
      memset(str, 0, sizeof(str));
      word_t vaddr_ifetch(vaddr_t addr, int len);
      vaddr_ifetch(iringbuf[i].pc, iringbuf[i].len);
      bool disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
      bool ret = disassemble(str, sizeof(str), iringbuf[i].pc, code, iringbuf[i].len);
      free(code);
      if (!ret) {
        printf("disassemble failed\n");
      } else {
        printf("%s\n", str);
      }
    }
  }
}

#endif

#endif
