
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
    ir_init();
}

static void cleanup(void)
{
    ir_destroy();
    node_destroy(root);
    stack_destroy(&stack);
    yylex_destroy();
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
    ir_find_globals(root);
    ir_obtain_all(root);
    ir_print_final(root);


    cleanup();
}
