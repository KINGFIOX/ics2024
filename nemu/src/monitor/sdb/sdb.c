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

#include "sdb.h"

#include <cpu/cpu.h>
#include <isa.h>
#include <readline/history.h>
#include <readline/keymaps.h>
#include <readline/readline.h>
#include <stdio.h>
#include <string.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {  // 在读取新行之前释放旧行
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) " /*prompt*/);
  // readline 可以识别 tab, ↑, ↓, ←, →, 回车. 不像 xv6 需要自己实现
  // readline 并不会清除前面的空格, 因此要有 strtok(str, " ")
  // 这里即使输入了空行, line_read 也不会为 NULL

  if (line_read && *line_read) {
    // it will help the user conveniently use the ↑ and ↓ to recall the previous command
    add_history(line_read);  // add line_read to history
  }

  return line_read;
}

static int cmd_c(char *_) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *_);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);   // expression evaluation
static int cmd_w(char *args);   // set watchpoint
static int cmd_d(char *args);   // delete watchpoint
static int cmd_si(char *args);  // single step

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "single step", cmd_si},
    {"info", "show information about registers, watchpoints, etc.", cmd_info},
    {"x", "show the content of the memory", cmd_x},
    {"p", "evaluate the expression", cmd_p},
    {"w", "set watchpoint", cmd_w},
    {"d", "delete watchpoint", cmd_d},
};

static int cmd_si(char *arg) {
  // TODO:
  return 0;
}

// d WP_ID
// example: d 2
static int cmd_d(char *args) {
  // TODO:
  return 0;
}

// w EXPR
// example: w *0x2000
static int cmd_w(char *args) {
  // TODO:
  return 0;
}

// p EXPR
// example: p $eax + 1
static int cmd_p(char *args) {
  // TODO:
  return 0;
}

// x N EXPR
// example: x 10 $esp
// 以十六进制输出连续的 N 个 4 字节
static int cmd_x(char *args) {
  // TODO:
  return 0;
}

// info [r|w]
// - r : registers
// - w : watchpoints
static int cmd_info(char *args) {
  char *arg = args;
  int argc = 0;
  arg = strtok(arg, " ");
  while (arg != NULL) {
    argc++;
    printf("arg: %s\n", arg);
    arg = strtok(NULL, " ");
  }
  if (argc != 1) {
    printf("Unknown command '%s'\n", args);
  }
  return 0;
}

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  printf("arg: %s\n", arg);
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
  if (is_batch_mode) {  // batch mode
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;  // 空行, 或者一行全是 space
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    // <cmd> <arg1> <arg2> ..., 这里 args 指向 <arg1>
    char *args = cmd + strlen(cmd) /*skip <cmd>*/ + 1 /*skip space before <arg1>*/;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
