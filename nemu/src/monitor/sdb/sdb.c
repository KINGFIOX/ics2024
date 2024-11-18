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

#include "common.h"
#include "memory/vaddr.h"
#include "watchpoint.h"

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

// handler return -1 means: exit the main loop
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

#define NR_CMD ARRLEN(cmd_table)

static int cmd_si(char *arg) {
  // TODO:
  cpu_exec(1);
  return 0;
}

// d WP_ID
// example: d 2
static int cmd_d(char *args) {
  // TODO:
  int no = atoi(args);
  WP *wp = find_wp(no);
  if (wp == NULL) {
    printf("Watchpoint %d not found\n", no);
    return 0;
  } else if (!wp->valid) {
    printf("Watchpoint %d is not set\n", no);
    return 0;
  }
  wp->valid = false;
  wp->last_value = 0;
  free_wp(wp);
  return 0;
}

// w EXPR
// example: w *0x2000
static int cmd_w(char *args) {
  // TODO:
  if (args == NULL) {
    printf("No expression\n");
    return 0;
  }
  if (strlen(args) >= WP_EXPR_LEN) {
    printf("Expression too long\n");
    return 0;
  }
  bool success;
  word_t value = expr(args, &success);
  if (!success) {
    printf("Invalid expression\n");
    return 0;
  }
  WP *wp = new_wp();
  if (wp == NULL) {
    printf("No free watchpoint\n");  // FIXME: log warning
    return 0;
  }
  wp->last_value = value;
  strncpy(wp->expr, args, WP_EXPR_LEN);
  wp->valid = true;
  return 0;
}

// p EXPR
// example: p $eax + 1
static int cmd_p(char *args) {
  bool success;
  word_t result = expr(args, &success);
  if (!success) {
    printf("Invalid expression\n");
    return 0;
  }
  printf("0x%x\n", result);
  return 0;
}

// x N EXPR
// example: x 10 $esp
// 以十六进制输出连续的 N 个 4 字节
static int cmd_x(char *args) {
  // TODO:
  args = strtok(NULL, " ");
  int nr = atoi(args);
  if (nr <= 0) {
    printf("Invalid number of bytes to display\n");
    return 0;
  }
  args = strtok(NULL, " ");
  bool success;
  vaddr_t base = expr(args, &success);
  if (!success) {
    printf("Invalid expression\n");
    return 0;
  }
  for (int i = 0; i < nr; i++) {
    vaddr_t addr = base + 4 * i;
    word_t value = vaddr_read(addr, 4);
    printf("0x%x\t0x%x\n", addr, value);
  }

  return 0;
}

// info [r|w]
// - r : registers
// - w : watchpoints
static int cmd_info(char *arg) {
  if (arg == NULL) {
    for (int i = 0; i < NR_CMD; i++) {
      if (0 == strcmp("info", cmd_table[i].name)) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
  } else if (0 == strcmp(arg, "r")) {
    isa_reg_display();
#ifdef CONFIG_FTRACE
  } else if (0 == strcmp(arg, "ftrace")) {
    void call_stack_dump(void);
    call_stack_dump();
#endif
#ifdef CONFIG_ITRACE
  } else if (0 == strcmp(arg, "itrace")) {
    void print_iringbuf(void);
    print_iringbuf();
#endif
#ifdef CONFIG_MTRACE
  } else if (0 == strcmp(arg, "mtrace")) {
    void mtrace_dump(void);
    mtrace_dump();
#endif
  } else if (0 == strcmp(arg, "w")) {
    watchpoint_display();
  } else {
    for (int i = 0; i < NR_CMD; i++) {
      if (0 == strcmp(arg, cmd_table[i].name)) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
  }
  return 0;
}

static int cmd_help(char *arg) {
  /* extract the first argument */

  if (arg == NULL) {  // 打印所有的命令
    /* no argument given */
    for (int i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {  // 打印指定命令的信息
    for (int i = 0; i < NR_CMD; i++) {
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

static char *last_str;
static int last_i;
static char *last_args;

void sdb_mainloop() {
  if (is_batch_mode) {  // batch mode
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    if (strlen(str) == 0 || last_str[0] != '\0') {
      str = last_str;
      printf("%s", str);
      if (cmd_table[last_i].handler(last_args) < 0) {
        return;
      }
      continue;
    }

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
          return;  // execute error happened
        }
        if (last_args != NULL) {
          free(last_args);
        }
        if (last_str != NULL) {
          free(last_str);
        }
        if (str != NULL) {
          last_str = strdup(str);
        }
        if (last_args != NULL) {
          last_args = strdup(args);
        }
        last_i = i;
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
