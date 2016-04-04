
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
#include "cgen.h"

/*******************************************************************************
*       Globals
*******************************************************************************/

pNode root;
Stack stack;

/*******************************************************************************
*       Main
*******************************************************************************/

__attribute__((constructor))
static void init(void)
{
    stack_init(&stack);
    ir_init();
}


__attribute__((destructor))
static void cleanup(void)
{
    ir_destroy();
    node_destroy(root);
    stack_destroy(&stack);
    yylex_destroy();
}


int main(void)
{
    // Parse yacc
    yyparse();

    // Simplify the node tree
    node_simplify(root);
    //node_print(root);

    // 1. put the global names in the global sym table
    // 2. Go through global sym table and resolve all bindings
    ir_find_globals(root);
    ir_obtain_all(root);
    //ir_print_final(root);

    // Generate code
    cgen_program();
}
