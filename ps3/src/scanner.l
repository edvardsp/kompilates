
%{
#include <stdio.h>

#include "vslc.h"
#include "nodetypes.h"
%}
%option noyywrap
%option array
%option yylineno

digit       [0-9]
digits      {digit}+
letter      [a-zA-Z]
letters     {letter}+

%%
FUNC                    { return FUNC;       }
BEGIN                   { return OPENBLOCK;  }
END                     { return CLOSEBLOCK; }
RETURN                  { return RETURN;     }
PRINT                   { return PRINT;      }
CONTINUE                { return CONTINUE;   }
IF                      { return IF;         }
THEN                    { return THEN;       }
ELSE                    { return ELSE;       }
WHILE                   { return WHILE;      }
DO                      { return DO;         }
VAR                     { return VAR;        }

[\:\+\-\*\/\=\<\>\(\)\,]    { return yytext[0]; }

_?{letter}({letter}|{digit}|_)*       { return IDENT;  }
{digits}                              { return NUMBER; }
\"(\\.|[^"])*\"                       { return STRING; }


\/\/.*\n                { /* Skip comment */ }
[ \t\n\r\v]             { /* Skip whitespace */ }
.                       { printf("Unkown character [%c]\n", yytext[0]); }

%%