%{

#include <stdio.h>

#include "memory/vaddr.h"

extern bool yy_success;
extern word_t yy_result;
extern const char *yy_err_msg;
extern int current_token;

int yylex(void);
void yyerror(const char *s);
int yyparse(void);

extern word_t vaddr_read(vaddr_t addr, int len);

%}

%left '+' '-'
%left '*' '/'
%left TK_NE_ TK_EQ_
%left TK_GT_ TK_GE_ TK_LT_ TK_LE_
%left TK_OR_ TK_AND_
%right UMINUS
%right DEREF

%token <num> TK_NUM_ TK_REG_
%type <num> expression logic_or logic_and equality comparison term factor unary primary

%union {
    word_t num;
}

%%

expression:
    logic_or { yy_result = $1; }
    ;

logic_or:
    logic_and { $$ = $1; }
    | logic_or TK_OR_ logic_and { $$ = ($1 || $3); }
    ;

logic_and:
    equality { $$ = $1; }
    | logic_and TK_AND_ equality { $$ = ($1 && $3); }
    ;

equality:
    comparison
    | equality TK_NE_ comparison  /* != */ { $$ = ($1 != $3); }
    | equality TK_EQ_ comparison  /* == */ { $$ = ($1 == $3); }
    ;

comparison:
    term { $$ = $1; }
    | comparison TK_GT_ term  /* > */ { $$ = ($1 > $3); }
    | comparison TK_GE_ term  /* >= */ { $$ = ($1 >= $3); }
    | comparison TK_LT_ term  /* < */ { $$ = ($1 < $3); }
    | comparison TK_LE_ term  /* <= */ { $$ = ($1 <= $3); }
    ;

term:
    factor { $$ = $1; }
    | term '-' factor { $$ = ($1 - $3); }
    | term '+' factor { $$ = ($1 + $3); }
    ;

factor:
    unary { $$ = $1; }
    | factor '*' unary { $$ = ($1 * $3); }
    | factor '/' unary {
        if ($3 == 0) {
            yyerror("divide by zero");
        } else {
            $$ = ($1 / $3);
        }
    }
    ;

unary:
    '-' unary %prec UMINUS { $$ = -$2; }
    | '*' unary %prec DEREF { $$ = vaddr_read($2, 4); }
    | primary { $$ = $1; }
    ;

primary:
    TK_NUM_ { $$ = $1; }
    | TK_REG_ { $$ = $1; }
    | '(' expression ')' { $$ = $2; }
    ;

%%

