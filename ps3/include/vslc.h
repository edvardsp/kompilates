#ifndef VSLC_H
#define VSLC_H

#include "nodetypes.h"
#include "ir.h"
#include "stack.h"
#include "y.tab.h"

int yyerror(const char *error);

extern int yylineno;
extern int yylex(void);
extern int yylex_destroy(void);
extern char yytext[];

extern pNode root;
extern Stack stack;

#endif
