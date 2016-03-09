
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
    destroy_symtab();
    stack_destroy(&stack);
}

int main(void)
{
    init();

    // Parse yacc
    yyparse();

    // Simplify the node tree
    node_simplify(root);

    // put the global names in the global sym table
    find_globals();

    // Iterate over all global symbols, resolve uses of variables:
    // Obtain all global names
    size_t n_globals = tlhash_size(global_names);
    pSymbol *global_list = malloc(sizeof(pSymbol) * n_globals);
    tlhash_values(global_names, (void **)&global_list);

    // Call bind_names on all those which are functions
    for (size_t i = 0; i < n_globals; i++)
        if (global_list[i]->type == SYM_FUNCTION)
            bind_names(global_list[i], global_list[i]->node);
    free(global_list);

    // Print the final state of the symbol table(s)
    print_symbols();
    printf("Bindings:\n");
    print_bindings(root);

    cleanup();
}
