#include <stdio.h>

#include "expr.h"
#include "expr.tab.h"

bool yy_success = true;
word_t yy_result = 0;
const char* yy_err_msg = NULL;
int current_token = 0;

int yylex(void) {
  Token tok = tokens[current_token];
  current_token++;
  if (0 == tok.type) {
    return 0;
  }
  switch (tok.type) {
    case TK_NUM:
      yylval.num = tok.val;
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

    case TK_OR:
      return TK_OR_;
    case TK_AND:
      return TK_AND_;

    default:  // + - * / ( )
      Assert('+' == tok.type || '-' == tok.type || '*' == tok.type || '/' == tok.type || '(' == tok.type || ')' == tok.type, "invalid token: %c", tok.type);
      return tok.type;
  }
}

void yyerror(const char* s) {
  yy_success = false;
  yy_err_msg = s;
}
