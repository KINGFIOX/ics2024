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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdio.h>

#include "debug.h"

enum {
  TK_NOTYPE = 256,  // 256 是因为: 正好超过了 char 的范围
  TK_EQ,
  TK_NUM,
  TK_REG,

  /* TODO: Add more token types */

};

// NOTE: 这里比较有趣, C 语言里的正则表达式, 要使用, 转义要使用 \\, 第一个 \ 是 C 语言里面的转义, 第二个 \ 是正则表达式里面的转义

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},   // spaces
    {"\\+", '+'},        // plus
    {"==", TK_EQ},       // equal
    {"-", '-'},          // minus
    {"\\*", '*'},        // multiply
    {"/", '/'},          // divide
    {"\\(", '('},        // left parenthesis
    {"\\)", ')'},        // right parenthesis
    {"[0-9]+", TK_NUM},  // number

    {"$eax|$ecx|$edx|$ebx|$esp|$ebp|$esi|$edi|$ax|$cx|$dx|$bx|$sp|$bp|$si|$di|$al|$cl|$dl|$bl|$ah|$ch|$dh|$bh", TK_REG},  // register

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

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;  // number of token

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NUM:
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].type = TK_NUM;
            nr_token++;
            break;
          case '+':
          case '-':
          case '*':
          case '/':
          case '(':
          case ')':
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_EQ:
            TODO();
          case TK_NOTYPE:
            // do nothing
            break;
          case TK_REG:
            printf("TK_REG: %s\n", substr_start);
            TODO();
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

static bool check_parentheses_sanity(void) {
  // TODO: the message of parentheses
  // position of parentheses is not matched

  int parentheses_count = 0;
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '(') {
      parentheses_count++;
    } else if (tokens[i].type == ')') {
      parentheses_count--;
      if (parentheses_count < 0) {  // (4 + 3)) * ((2 - 1)
        return false;
      }
    }
  }
  return parentheses_count == 0;
}

static bool check_parentheses(int p, int q) {
  Assert((p < q) && (p >= 0) && (q <= nr_token - 1), "parentheses position is invalid: p=%d q=%d, nr_token=%d", p, q, nr_token);
  return tokens[p].type == '(' && tokens[q].type == ')';
}

static word_t eval(int p, int q) {
  if (p > q) {
    // TODO: empty
    return 0;
  } else if (check_parentheses(p, q)) {
    return eval(p + 1, q - 1);
  } else {
  }
  TODO();
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if (!check_parentheses_sanity()) {
    *success = false;
    return 0;
  }

  return eval(0, nr_token - 1);
}
