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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include "expr.h"

#include <regex.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "debug.h"
#include "macro.h"
#include "memory/vaddr.h"

// NOTE: 这里比较有趣, C 语言里的正则表达式, 要使用, 转义要使用 \\, 第一个 \ 是 C 语言里面的转义, 第二个 \ 是正则表达式里面的转义

// expression     → equality ;
// equality       → comparison ( ( "!=" | "==" ) comparison )* ;
// comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
// term           → factor ( ( "-" | "+" ) factor )* ;
// factor         → unary ( ( "/" | "*" ) unary )* ;
// unary          → ( "!" | "-" ) unary
//                | primary ;
// primary        → NUMBER | STRING | "true" | "false" | "nil"
//                | "(" expression ")" ;

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    // FIXME: 这里有瑕疵, 或许应该要推倒语法分析处理
    {"\\*0x[0-9a-fA-F]+", TK_MEM},  // highest precedence
    {"\\*($eax|\\$ecx|\\$edx|\\$ebx|\\$esp|\\$ebp|\\$esi|\\$edi|\\$ax|\\$cx|\\$dx|\\$bx|\\$sp|\\$bp|\\$si|\\$di|\\$al|\\$cl|\\$dl|\\$bl|\\$ah|\\$ch|\\$dh|\\$"
     "bh)",
     TK_MEM},  // register

    {" +", TK_NOTYPE},  // spaces

    {"\\+", '+'},  // plus
    {"-", '-'},    // minus
    {"\\*", '*'},  // multiply
    {"\\/", '/'},  // divide

    {"!=", TK_NE},  // not equal
    {"==", TK_EQ},  // equal

    {">=", TK_GE},  // greater than or equal
    {"<=", TK_LE},  // less than or equal
    {">", TK_GT},   // greater than
    {"<", TK_LT},   // less than

    {"\\(", '('},  // left parenthesis
    {"\\)", ')'},  // right parenthesis

    {"([0-9]+)|(0x[0-9a-fA-F]+)", TK_NUM},  // number

    {"\\$eax|\\$ecx|\\$edx|\\$ebx|\\$esp|\\$ebp|\\$esi|\\$edi|\\$ax|\\$cx|\\$dx|\\$bx|\\$sp|\\$bp|\\$si|\\$di|\\$al|\\$cl|\\$dl|\\$bl|\\$ah|\\$ch|\\$dh|\\$bh",
     TK_REG},  // register

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  char error_msg[128];

  for (int i = 0; i < NR_REGEX; i++) {
    // compile the regex into DFA
    int ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

struct {
  int type;
  char str[32];
} __tokens[32] __attribute__((used)) = {};
int __nr_token __attribute__((used)) = 0;  // number of token

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:  // do nothing, skip
            break;

          case '+':
          case '-':
          case '*':
          case '/':

          case TK_EQ:
          case TK_NE:
          case TK_GT:
          case TK_GE:
          case TK_LT:
          case TK_LE:

          case '(':
          case ')':

          case TK_REG:
          case TK_MEM:
          case TK_NUM:
            strncpy(__tokens[__nr_token].str, substr_start, substr_len);
            __tokens[__nr_token].type = rules[i].token_type;
            __nr_token++;
            break;
          default:
            TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

Token tokens[32] = {};
int nr_token = 0;  // number of token

static bool make_value(void) {
  nr_token = __nr_token;
  for (int i = 0; i < nr_token; i++) {
    tokens[i].type = __tokens[i].type;
    if (TK_NUM == __tokens[i].type) {  // number
      if (0 == strncmp(__tokens[i].str, "0x", 2)) {
        tokens[i].val = strtol(__tokens[i].str, NULL, 16);
      } else {
        tokens[i].val = atoi(__tokens[i].str);
      }
    } else if (TK_REG == __tokens[i].type) {  // register
      const char *reg = __tokens[i].str + 1;  // skip the '$'
      tokens[i].type = TK_NUM;
      bool success;
      tokens[i].val = isa_reg_str2val(reg, &success);
      if (!success) {
        panic("impossible, str: %s", __tokens[i].str);
      }
    } else if (TK_MEM == __tokens[i].type) {
      const char *mem = __tokens[i].str + 1;  // skip the '*'
      if ('$' == mem[0]) {
        const char *reg = mem + 1;  // skip the '$'
        bool success;
        vaddr_t addr = isa_reg_str2val(reg, &success);
        if (!success) {
          panic("impossible, str: %s, reg: %s", __tokens[i].str, reg);
        }
        tokens[i].val = vaddr_read(addr, 4);
        tokens[i].type = TK_NUM;
      } else if (0 == strncmp(mem, "0x", 2)) {
        vaddr_t addr = strtol(mem, NULL, 16);
        tokens[i].val = vaddr_read(addr, 4);
        tokens[i].type = TK_NUM;
      } else {
        panic("impossible, str: %s, mem[0]: %c", __tokens[i].str, mem[0]);
      }
    }
  }
  return true;
}

word_t expr(char *e, bool *success) {
  // clear the __tokens
  __nr_token = 0;
  for (int i = 0; i < ARRLEN(__tokens); i++) {
    __tokens[i].type = 0;
    memset(__tokens[i].str, 0, 32);
  }

  // clear the tokens
  nr_token = 0;
  for (int i = 0; i < ARRLEN(tokens); i++) {
    tokens[i].type = 0;
    tokens[i].val = 0;
  }

  // lex the expr
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  // check the __tokens
  for (int i = 0; i < __nr_token; i++) {
    Assert(__tokens[i].type != 0 && __tokens[i].type != TK_NOTYPE, "invalid token");
  }

  // canonicalize the tokens
  if (!make_value()) {
    *success = false;
    return 0;
  }

  // check the tokens
  for (int i = 0; i < nr_token; i++) {
    Assert(tokens[i].type != 0 && tokens[i].type != TK_NOTYPE && tokens[i].type != TK_REG, "invalid token");
  }

  // clear the state of bison
  current_token = 0;
  yy_success = true;
  yy_err_msg = NULL;
  yy_result = 0;

  if (0 != yyparse()) {
    *success = false;
    printf("yyparse error: %s\n", yy_err_msg);
    return 0;
  }

  *success = true;
  return yy_result;
}
