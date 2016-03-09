#ifndef IR_H
#define IR_H

#include <stddef.h>

#include "node.h"
#include "tlhash.h"

typedef enum {
    SYM_GLOBAL_VAR, SYM_FUNCTION, SYM_PARAMETER, SYM_LOCAL_VAR
} Symtype;

typedef struct s {
    char *name;
    Symtype type;
    pNode node;
    size_t seq;
    size_t nparms;
    pTlhash locals;
} Symbol;

typedef Symtype* pSymtype;
typedef Symbol* pSymbol;

extern pTlhash global_names;
extern char **string_list;
extern size_t n_string_list;
extern size_t stringc;

void find_globals  (void);
void bind_names    (pSymbol function, pNode root);
void print_symbols (void);
void print_bindings(pNode root);
void destroy_symtab(void);

#endif
