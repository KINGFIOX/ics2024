#include "expr.h"

bool yy_success = true;
word_t yy_result = 0;
const char* yy_err_msg = NULL;
int current_token = 0;

int yylex(void) {
  Token tok = tokens[current_token];
  current_token++;
  if (tok.type == TK_NUM) {
    return tok.val;
  }
  return tok.type;
}

void yyerror(const char* s) {
  yy_success = false;
  yy_err_msg = s;
}
