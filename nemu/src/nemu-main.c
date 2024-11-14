/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <common.h>
#include <signal.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

void sig_handler(int signo, siginfo_t *info, void *ucontext) {
  if (signo == SIGABRT) {
#ifdef CONFIG_ITRACE
    void print_iringbuf(void);
    print_iringbuf();
    void print_pc();
    print_pc();
#endif
#ifdef CONFIG_MTRACE
    void mtrace_dump(void);
    mtrace_dump();
#endif
    // could not use ftrace here, because Assert in it
    // #ifdef CONFIG_FTRACE
    //     void call_stack_dump();
    //     call_stack_dump();
    // #endif
  }
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  /* catch SIGABRT. */
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = &sig_handler;
  sa.sa_flags = 0;
  sigaction(SIGABRT, &sa, NULL);

  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  // #ifdef CONFIG_FTRACE
  //   void call_stack_dump();
  //   call_stack_dump();
  // #endif

  return is_exit_status_bad();
}
