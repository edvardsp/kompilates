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

void ir_find_globals  (void);
void ir_bind_names    (pSymbol function, pNode root);
void ir_print_symbols (void);
void ir_print_bindings(pNode root);
void ir_symtab_destroy(void);
void ir_obtain_all    (void);
void ir_print_final   (pNode root);

#endif
