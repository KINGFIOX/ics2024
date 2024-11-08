#ifndef __EXPR_H__
#define __EXPR_H__

#include <common.h>
#include <isa.h>

#include "debug.h"
#include "macro.h"

typedef struct token {
  int type;
  char str[32];
} Token;

#endif