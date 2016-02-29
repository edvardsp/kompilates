#ifndef VSLC_H
#define VSLC_H

/*******************************************************************************
*       Includes
*******************************************************************************/

#include "nodetypes.h"
#include "ir.h"
#include "stack.h"
#include "y.tab.h"

/*******************************************************************************
*       Functions
*******************************************************************************/

int yyerror(const char *error);

/*******************************************************************************
*       Globals
*******************************************************************************/

extern int yylineno;
extern int yylex(void);
extern int yylex_destroy(void);
extern char yytext[];

extern pNode root;
extern Stack stack;

#endif
