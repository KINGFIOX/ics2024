#include "expr.h"

int current_token = 0;

int yylex(void) {
  Token tok = tokens[current_token];
  current_token++;
  if (tok.type == TK_NUM) {
    return tok.val;
  }
  return tok.type;
}