
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "vslc.h"
#include "nodetypes.h"
#include "stack.h"

pNode root;
Stack stack;

int main(void)
{
    // Initialize stack
    stack_init(&stack);

    // Parse yacc
    yyparse();

    // Simplify the node tree and print it
    node_simplify(root);
    node_print(root, 0);

    // Clean up
    yylex_destroy();
    node_destroy(root);
    stack_destroy(&stack);
}
