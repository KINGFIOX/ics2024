#ifndef __EXPR_H__
#define __EXPR_H__

#include <common.h>
#include <isa.h>

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

typedef struct token {
  int type;
  word_t val;
} Token;

extern Token tokens[];
extern int nr_token;

extern bool yy_success;
extern word_t yy_result;
extern const char *yy_err_msg;
extern int current_token;

int yylex(void);
void yyerror(const char *s);
extern int yyparse(void);

#endif