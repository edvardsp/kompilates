%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nodetypes.h"
#include "vslc.h"
#include "node.h"
%}
%expect 1

%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK
%token VAR IDENT NUMBER STRING

%%

program : global_list { node_reduce(PROGRAM, NULL, 1);
                        root = stack_pop(&stack); }
        ;

global_list : global             { node_reduce(GLOBAL_LIST, NULL, 1); }
            | global_list global { node_reduce(GLOBAL_LIST, NULL, 2); }
            ;

global : function    { node_reduce(GLOBAL, NULL, 1); }
       | declaration { node_reduce(GLOBAL, NULL, 1); }
       ;

statement_list : statement                { node_reduce(STATEMENT_LIST, NULL, 1); }
               | statement_list statement { node_reduce(STATEMENT_LIST, NULL, 2); }
               ;

print_list : print_item                { node_reduce(PRINT_LIST, NULL, 1); }
           | print_list ',' print_item { node_reduce(PRINT_LIST, NULL, 2); }
           ;

expression_list : expression                     { node_reduce(EXPRESSION_LIST, NULL, 1); }
                | expression_list ',' expression { node_reduce(EXPRESSION_LIST, NULL, 2); }
                ;

variable_list : identifier                   { node_reduce(VARIABLE_LIST, NULL, 1); }
              | variable_list ',' identifier { node_reduce(VARIABLE_LIST, NULL, 2); }
              ;

argument_list : expression_list { node_reduce(ARGUMENT_LIST, NULL, 1); }
              | /* empty */     { stack_push(&stack, NULL);            }
              ;

parameter_list : variable_list { node_reduce(PARAMETER_LIST, NULL, 1); }
               | /* empty */   { stack_push(&stack, NULL);             }
               ;

declaration_list : declaration                  { node_reduce(DECLARATION_LIST, NULL, 1); }
                 | declaration_list declaration { node_reduce(DECLARATION_LIST, NULL, 2); }
                 ;

function : FUNC identifier '(' parameter_list ')' block { node_reduce(FUNCTION, NULL, 3); }
         ;

statement : assignment_statement { node_reduce(STATEMENT, NULL, 1); }
          | return_statement     { node_reduce(STATEMENT, NULL, 1); }
          | print_statement      { node_reduce(STATEMENT, NULL, 1); }
          | if_statement         { node_reduce(STATEMENT, NULL, 1); }
          | while_statement      { node_reduce(STATEMENT, NULL, 1); }
          | null_statement       { node_reduce(STATEMENT, NULL, 1); }
          | block                { node_reduce(STATEMENT, NULL, 1); }
          ;

block : OPENBLOCK declaration_list statement_list CLOSEBLOCK { node_reduce(BLOCK, NULL, 2); }
      | OPENBLOCK statement_list CLOSEBLOCK                  { node_reduce(BLOCK, NULL, 1); }
      ;

assignment_statement : identifier ':' '=' expression             { node_reduce(ASSIGNMENT_STATEMENT, NULL, 2); }
                     ;
return_statement     : RETURN expression                         { node_reduce(RETURN_STATEMENT, NULL, 1); }
                     ;
print_statement      : PRINT print_list                          { node_reduce(PRINT_STATEMENT, NULL, 1); }
                     ;
null_statement       : CONTINUE                                  { node_reduce(NULL_STATEMENT, NULL, 0); }
                     ;
if_statement         : IF relation THEN statement                { node_reduce(IF_STATEMENT, NULL, 2); }
                     | IF relation THEN statement ELSE statement { node_reduce(IF_STATEMENT, NULL, 3); }
                     ;
while_statement      : WHILE relation DO statement               { node_reduce(WHILE_STATEMENT, NULL, 2); }
                     ;

relation : expression '=' expression { node_reduce(RELATION, strdup("="), 2); }
         | expression '<' expression { node_reduce(RELATION, strdup("<"), 2); }
         | expression '>' expression { node_reduce(RELATION, strdup(">"), 2); }
         ;

expression : expression '+' expression        { node_reduce(EXPRESSION, strdup("+"), 2);  }
           | expression '-' expression        { node_reduce(EXPRESSION, strdup("-"), 2);  }
           | expression '*' expression        { node_reduce(EXPRESSION, strdup("*"), 2);  }
           | expression '/' expression        { node_reduce(EXPRESSION, strdup("/"), 2);  }
           | '-' expression %prec UMINUS      { node_reduce(EXPRESSION, strdup("-"), 1);  }
           | '(' expression ')'               { /* Skip nesting parenthesis */    }
           | number                           { node_reduce(EXPRESSION, NULL, 1); }
           | identifier                       { node_reduce(EXPRESSION, NULL, 1); }
           | identifier '(' argument_list ')' { node_reduce(EXPRESSION, NULL, 2); }
           ;

declaration : VAR variable_list { node_reduce(DECLARATION, NULL, 1); }
            ;

print_item : expression { node_reduce(PRINT_ITEM, NULL, 1); }
           | string     { node_reduce(PRINT_ITEM, NULL, 1); }
           ;

identifier : IDENT { node_reduce(IDENTIFIER_DATA, strdup(yytext), 0); }
           ;
number : NUMBER {
           int *buf = malloc(sizeof(int));
           *buf = strtol(yytext, NULL, 10);
           node_reduce(NUMBER_DATA, buf, 0);
         }
       ;
string : STRING { node_reduce(STRING_DATA, strdup(yytext), 0); }
       ;

%%

int yyerror(const char *error)
{
    fprintf(stderr, "%s on line %d\n", error, yylineno);
    exit(EXIT_FAILURE);
}
