#ifndef __EXPR_H__
#define __EXPR_H__

#include <common.h>
#include <isa.h>

typedef struct token {
  int type;
  char str[32];
} Token;

extern Token tokens[];
extern int nr_token;

enum {
  TK_NOTYPE = 256,  // 256 是因为: 正好超过了 char 的范围

  TK_EQ,
  TK_NE,
  TK_GE,
  TK_LE,
  TK_GT,
  TK_LT,

  TK_NUM,
  TK_REG,
  /* TODO: Add more token types */

};

#endif