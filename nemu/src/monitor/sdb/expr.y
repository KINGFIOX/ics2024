%{

#include <stdio.h>

#include "expr.h"
#include "memory/vaddr.h"

%}

%left '+' '-'
%left '*' '/'
%left TK_NE_ TK_EQ_
%left TK_GT_ TK_GE_ TK_LT_ TK_LE_
%right UMINUS
%right DEREF

%token <num> TK_NUM_
%type <num> expression equality comparison term factor unary primary

%union {
    word_t num;
}

%%

expression:
    equality { yy_result = $1; }
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
    | '(' expression ')' { $$ = $2; }
    ;

%%

