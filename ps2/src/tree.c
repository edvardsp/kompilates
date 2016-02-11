
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

#include "vslc.h"


node_t* node_create()
{
    node_t *out = (node_t *)malloc(sizeof(node_t));
    if (!out)
    {
        
        fprintf(stderr, "malloc of create_node failed\n");
        exit(1);
    }
    return out;
}


void node_print(node_t *root, int nesting)
{
    if (root != NULL)
    {
        /* Print the type of node indented by the nesting level */
        printf("%*c%s", nesting, ' ', node_string[root->type]);

        /* For identifiers, strings, expressions and numbers,
         * print the data element also
         */
        if (root->type == IDENTIFIER_DATA ||
            root->type == STRING_DATA ||
            root->type == EXPRESSION) 
            printf("(%s)", (char *)root->data);
        else if (root->type == NUMBER_DATA)
            printf("(%ld)", *(int64_t *)root->data);

        /* Make a new line, and traverse the node's children in the same manner */
        putchar('\n');
        for (int64_t i = 0; i < root->n_children; i++)
            node_print(root->children[i], nesting + 1);
    }
    else
        printf("%*c%p\n", nesting, ' ', root);
}


/* Take the memory allocated to a node and fill it in with the given elements */
void node_init(node_t *nd, node_index_t type, void *data, uint64_t n_children, ...)
{
    va_list argp;
    va_start(argp, n_children);

    nd->type = type;
    nd->data = data;
    nd->entry = NULL;
    nd->n_children = n_children;
    nd->children = (node_t **)malloc(n_children * sizeof(node_t *));
    for (uint64_t i = 0; i < n_children; i++)
    {
        nd->children[i] = va_arg(argp, node_t *);
    }

    va_end(argp);
}


static void __reduce_1(node_t *node, node_index_t type, void *data)
{
    node_t *ch1 = stack_pop(&stack);
    node_init(node, type, data, 1, ch1);
}

static void __reduce_2(node_t *node, node_index_t type, void *data)
{
    node_t *ch1 = stack_pop(&stack);
    node_t *ch2 = stack_pop(&stack);
    node_init(node, type, data, 2, ch2, ch1);
}


static void __reduce_3(node_t *node, node_index_t type, void *data)
{
    node_t *ch1 = stack_pop(&stack);
    node_t *ch2 = stack_pop(&stack);
    node_t *ch3 = stack_pop(&stack);
    node_init(node, type, data, 3, ch3, ch2, ch1);
}

void node_reduce(node_index_t type, void *data, int n_pops)
{
    node_t *node = node_create();  
    switch (n_pops)
    {
        case 0: node_init(node, type, data, 0); break;
        case 1: __reduce_1(node, type, data);   break;
        case 2: __reduce_2(node, type, data);   break;
        case 3: __reduce_3(node, type, data);   break;
        default:
            fprintf(stderr, "node_reduce called with n=%i\n", n_pops);
            exit(1);
    }
    stack_push(&stack, node); 
}


/* Remove a node and its contents */
void node_finalize(node_t *discard)
{
    assert(discard->children == NULL);

    free(discard->data);
    free(discard->entry);
    free(discard);
}


/* Recursively remove the entire tree rooted at a node */
void destroy_subtree(node_t *discard)
{
    if (!discard)
        return;
    
    if (discard->children)
    {
        for (uint64_t i = 0; i < discard->n_children; i++)
            destroy_subtree(discard->children[i]);
        free(discard->children);
        discard->children = NULL;
    }
    node_finalize(discard);
}
