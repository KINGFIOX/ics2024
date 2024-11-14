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
#include <cpu/difftest.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;  // global variable, number of guest instructions executed
static uint64_t g_timer = 0;   // unit: us
static bool g_print_step = true;

void device_update();

bool check_wp(void);

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) {
    log_write("%s\n", _this->logbuf);
  }
#endif
  if (g_print_step) {
    IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
  }
#ifdef CONFIG_WATCHPOINT
  if (check_wp()) {  // NOTE: 这里会出现: 同时触发两次 watchpoint 的情况
    nemu_state.state = NEMU_STOP;
  }
#endif
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
}

static bool exec_once(Decode *s, vaddr_t pc) {
  // printf("pc = %x\n", pc);
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;

#ifdef CONFIG_ITRACE

  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst;
  for (i = 0; i < ilen; i++) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  bool disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  bool ret = disassemble(p, s->logbuf + sizeof(s->logbuf) - p, MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst, ilen);
  if (!ret) {
    return false;
  }
#endif
  return true;
}

#ifdef CONFIG_ITRACE

#define ITRACE_BUF_SIZE 32
typedef struct {
  char str[128];
  vaddr_t pc;
} iringbuf_t;

static iringbuf_t iringbuf[ITRACE_BUF_SIZE];
static int rear = 0;

#endif

void print_iringbuf() {
  for (int i = (rear + 1) % ITRACE_BUF_SIZE; i != rear; i = (i + 1) % ITRACE_BUF_SIZE) {
    if (iringbuf[i].pc != 0) {
      printf("%s\n", iringbuf[i].str);
    }
  }
}

static void execute(uint64_t n) {
  Decode s;
  for (; n > 0; n--) {
    bool ret __attribute__((unused));
    ret = exec_once(&s, cpu.pc);
    if (!ret) {
      // print
      nemu_state.state = NEMU_ABORT;
    } else {
      // record
      strncpy(iringbuf[rear].str, s.logbuf, sizeof(iringbuf[rear].str));
      iringbuf[rear].pc = cpu.pc;
      rear = (rear + 1) % ITRACE_BUF_SIZE;
    }
    g_nr_guest_inst++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0)
    Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else
    Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END:
    case NEMU_ABORT:
    case NEMU_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default:
      nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);  // n is the number of instructions(cycles) to execute

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING:
      nemu_state.state = NEMU_STOP;
      break;

    case NEMU_END:
    case NEMU_ABORT:
      if (nemu_state.state == NEMU_ABORT) {
        print_iringbuf();
        void mtrace_dump();
        mtrace_dump();
      }
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED)
                                          : (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT:
      statistic();  // print the statistic information while quitting
  }
}

void print_pc() { printf("pc = %x\n", cpu.pc); }
