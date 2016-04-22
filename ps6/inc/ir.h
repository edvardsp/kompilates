#ifndef IR_H
#define IR_H

/*******************************************************************************
*       Includes
*******************************************************************************/

#include <stddef.h> // for size_t

#include "node.h"
#include "tlhash.h"

/*******************************************************************************
*       Types
*******************************************************************************/

typedef enum {
    SYM_GLOBAL_VAR,
    SYM_FUNCTION,
    SYM_PARAMETER,
    SYM_LOCAL_VAR
} Symtype;

typedef struct {
    char *name;
    Symtype type;
    pNode node;
    size_t seq;
    size_t nparms;
    pTlhash locals;
} Symbol, *pSymbol;

typedef struct {
    pTlhash global_names;
    char **string_list;
    size_t stringc;
} Ir, *pIr;

/*******************************************************************************
*       Globals
*******************************************************************************/

extern Ir ir;

/*******************************************************************************
*       Functions
*******************************************************************************/

void ir_init          (void);
void ir_find_globals  (pNode root);
void ir_bind_names    (pSymbol function, pNode root);
void ir_print_symbols (void);
void ir_print_bindings(pNode root);
void ir_destroy       (void);
void ir_obtain_all    (pNode root);
void ir_print_final   (pNode root);

#endif
