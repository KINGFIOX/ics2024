#include "expr.h"
#include "expr.tab.h"

bool yy_success = true;
word_t yy_result = 0;
const char* yy_err_msg = NULL;
int current_token = 0;

int yylex(void) {
  Token tok = tokens[current_token];
  current_token++;
  switch (tok.type) {
    case TK_NUM:
      return TK_NUM_;

    case TK_EQ:
      return TK_EQ_;
    case TK_NE:
      return TK_NE_;
    case TK_GE:
      return TK_GE_;
    case TK_LE:
      return TK_LE_;
    case TK_GT:
      return TK_GT_;
    case TK_LT:
      return TK_LT_;

    default:  // + - * / ( )
      return tok.type;
  }
}

void yyerror(const char* s) {
  yy_success = false;
  yy_err_msg = s;
}
