
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "vslc.h"


pNode node_create()
{
    pNode out = (pNode)calloc(1, sizeof(Node));
    if (!out)
    {
        
        fprintf(stderr, "calloc of create_node failed\n");
        exit(1);
    }
    return out;
}


void node_print(pNode root, int nesting)
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
            root->type == RELATION ||
            root->type == EXPRESSION) 
            printf("(%s)", (char *)root->data);
        else if (root->type == NUMBER_DATA)
            printf("(%i)", *(int *)root->data);

        /* Make a new line, and traverse the node's children in the same manner */
        putchar('\n');
        for (int i = 0; i < root->n_children; i++)
            node_print(root->children[i], nesting + 1);
    }
    else
        printf("%*c%p\n", nesting, ' ', root);
}


/* Take the memory allocated to a node and fill it in with the given elements */
void node_init(pNode nd, node_index_t type, void *data, int n_children, ...)
{
    va_list argp;
    va_start(argp, n_children);

    nd->type = type;
    nd->data = data;
    nd->entry = NULL;
    nd->n_children = n_children;
    nd->children = (pNode *)calloc(n_children, sizeof(pNode));
    for (int i = 0; i < n_children; i++)
    {
        nd->children[i] = va_arg(argp, pNode);
    }

    va_end(argp);
}


static void __reduce_1(pNode node, node_index_t type, void *data)
{
    pNode ch1 = stack_pop(&stack);
    node_init(node, type, data, 1, ch1);
}

static void __reduce_2(pNode node, node_index_t type, void *data)
{
    pNode ch1 = stack_pop(&stack);
    pNode ch2 = stack_pop(&stack);
    node_init(node, type, data, 2, ch2, ch1);
}


static void __reduce_3(pNode node, node_index_t type, void *data)
{
    pNode ch1 = stack_pop(&stack);
    pNode ch2 = stack_pop(&stack);
    pNode ch3 = stack_pop(&stack);
    node_init(node, type, data, 3, ch3, ch2, ch1);
}

void node_reduce(node_index_t type, void *data, int n_pops)
{
    pNode node = node_create();  
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
void node_finalize(pNode discard)
{
    assert(discard->children == NULL);

    free(discard->data);
    free(discard->entry);
    free(discard);
}


/* Recursively remove the entire tree rooted at a node */
void node_destroy(pNode discard)
{
    if (!discard)
        return;
    
    if (discard->children)
    {
        for (int i = 0; i < discard->n_children; i++)
            node_destroy(discard->children[i]);
        free(discard->children);
        discard->children = NULL;
    }
    node_finalize(discard);
}


void node_simplify(pNode *simplified, pNode parent)
{
    *simplified = parent;
}