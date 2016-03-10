
/*******************************************************************************
*       Includes
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "vslc.h"
#include "nodetypes.h"
#include "node.h"
#include "stack.h"
#include "ir.h"
#include "tlhash.h"

/*******************************************************************************
*       Globals
*******************************************************************************/

pNode root;
Stack stack;

/*******************************************************************************
*       Main
*******************************************************************************/

static void init(void)
{
    stack_init(&stack);
}

static void cleanup(void)
{
    yylex_destroy();
    node_destroy(root);
    ir_symtab_destroy();
    stack_destroy(&stack);
}

int main(void)
{
    init();

    // Parse yacc
    yyparse();

    // Simplify the node tree
    node_simplify(root);

    // 1. put the global names in the global sym table
    // 2. Obtain all global names
    // 3. Print the final state of the symbol table(s)
    ir_find_globals();
    ir_obtain_all();
    ir_print_final(root);


    cleanup();
}
