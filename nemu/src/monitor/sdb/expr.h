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

#endif