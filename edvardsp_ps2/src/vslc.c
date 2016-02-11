#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "vslc.h"
#include "nodetypes.h"
#include "stack.h"

node_t *root;
stack_t stack;

int main(void)
{
    stack_init(&stack);

    yyparse();
    node_print(root, 0);

    yylex_destroy();
    destroy_subtree(root);
    stack_destroy(&stack);
}
