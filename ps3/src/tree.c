
/*******************************************************************************
*       Includes
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "vslc.h"

/*******************************************************************************
*       Defines
*******************************************************************************/

#define TOINT(node) (*(int *)((node)->data))


#define SINGLE_TYPE(node) \
    ((node)->type == GLOBAL         || \
     (node)->type == STATEMENT      || \
     (node)->type == PRINT_ITEM     || \
     (node)->type == ARGUMENT_LIST  || \
     (node)->type == PARAMETER_LIST)


#define LIST_TYPE(node) \
    ((node)->type == PRINT_LIST      || \
     (node)->type == GLOBAL_LIST     || \
     (node)->type == VARIABLE_LIST   || \
     (node)->type == STATEMENT_LIST  || \
     (node)->type == EXPRESSION_LIST || \
     (node)->type == PRINT_STATEMENT || \
     (node)->type == DECLARATION_LIST)


#define EXPRDATA_TYPE(node) \
    ((node)->type == NUMBER_DATA || \
     (node)->type == IDENTIFIER_DATA)

/*******************************************************************************
*       Functions
*******************************************************************************/

/*******************************************************************************
*   node_create
*   |   Allocates memory for a node object 
*******************************************************************************/

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

/*******************************************************************************
*   node_print
*   |   Print out the node tree recurseively 
*******************************************************************************/

void node_print(pNode curr, int nesting)
{
    if (curr != NULL)
    {
        /* Print the type of node indented by the nesting level */
        printf("%*c%s", nesting, ' ', node_string[curr->type]);

        /* For identifiers, strings, expressions and numbers,
         * print the data element also
         */
        if (curr->type == IDENTIFIER_DATA ||
            curr->type == STRING_DATA ||
            curr->type == RELATION ||
            curr->type == EXPRESSION) 
            printf("(%s)", (char *)curr->data);
        else if (curr->type == NUMBER_DATA)
            printf("(%i)", *(int *)curr->data);

        /* Make a new line, and traverse the node's children in the same manner */
        putchar('\n');
        for (size_t i = 0; i < curr->n_children; i++)
            node_print(curr->children[i], nesting + 1);
    }
    else
        printf("%*c%p\n", nesting, ' ', curr);
}

/*******************************************************************************
*   node_init
*   |   Take the memory allocated to a node and fill it in 
*   |   with the given elements 
*******************************************************************************/

void node_init(pNode nd, node_index_t type, void *data, size_t n_children, ...)
{
    va_list argp;
    va_start(argp, n_children);

    nd->type = type;
    nd->data = data;
    nd->entry = NULL;
    nd->n_children = n_children;
    if (n_children > 0)
        nd->children = (pNode *)calloc(n_children, sizeof(pNode));
    for (size_t i = 0; i < n_children; i++)
    {
        nd->children[i] = va_arg(argp, pNode);
    }

    va_end(argp);
}

/*******************************************************************************
*   node_reduce
*   |   Used by parser, used to pop the stack, create new node 
*   |   with the popped as childs, push back created node.
*******************************************************************************/

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


void node_reduce(node_index_t type, void *data, size_t n_pops)
{
    pNode node = node_create();  
    switch (n_pops)
    {
        case 0: node_init(node, type, data, 0); break;
        case 1: __reduce_1(node, type, data);   break;
        case 2: __reduce_2(node, type, data);   break;
        case 3: __reduce_3(node, type, data);   break;
        default:
            fprintf(stderr, "node_reduce called with n=%zu\n", n_pops);
            exit(1);
    }
    stack_push(&stack, node); 
}

/*******************************************************************************
*   node_finalize
*   |   Remove a node and its contents 
*******************************************************************************/

void node_finalize(pNode discard)
{
    assert(discard != NULL);

    free(discard->data);
    free(discard->entry);
    free(discard->children);
    free(discard);
}

/*******************************************************************************
*   node_destroy
*   |   Recursively remove the entire tree rooted at a node 
*******************************************************************************/

void node_destroy(pNode discard)
{
    if (!discard)
        return;
    
    if (discard->children)
    {
        for (size_t i = 0; i < discard->n_children; i++)
            node_destroy(discard->children[i]);
    }
    node_finalize(discard);
}

/*******************************************************************************
*   node__simplify
*   |   Recursively simplify the entire node tree of redundant nodes 
*******************************************************************************/

static bool __simplify_single_node(pNode *child, pNode parent, size_t n)
{
    if (SINGLE_TYPE(*child))
    {
        assert((*child)->n_children == 1);

        // Update children array and free memory
        parent->children[n] = (*child)->children[0];
        node_finalize(*child);

        // Update the pointer
        *child = parent->children[n];

        return true;
    }
    return false;
}


static bool __simplify_lists(pNode current)
{
    if (!current || !LIST_TYPE(current))
        return false;

    // Find child with same type
    size_t n = 0;
    pNode next = NULL;
    for ( ; n < current->n_children; n++)
    {
        pNode same = current->children[n];
        if (same && !LIST_TYPE(same))
            continue;
        next = same;
        break;
    }
    if (!next)
        return false;

    // Do recursion first
    __simplify_lists(next);

    // Resize children buffer
    size_t new_size = current->n_children - 1 + next->n_children;
    pNode *buf1 = (pNode *)realloc(current->children, new_size * sizeof(pNode));
    if (!buf1)
    {
        printf("Realloc during simplfy lists failed\n");
        exit(1);    
    }
    current->children = buf1;

    // Extend gap of next node for possible more children
    size_t extend = next->n_children - 1;
    if (extend > 0 && n < current->n_children - 1)
    {
        size_t nbytes = sizeof(pNode) * (current->n_children - n - 1);
        pNode *buf2 = memmove(&current->children[n+1+extend], &current->children[n+1], nbytes);
        if (!buf2)
        {
            printf("Memmove during simplfy lists failed\n");
            exit(1);    
        }    
    }

    // Append new childs
    for (size_t i = 0; i < next->n_children; i++)
    {
        size_t index = n + i;
        current->children[index] = next->children[i];
    }
    current->n_children = new_size;

    // Free next node
    node_finalize(next);

    return true;
}


static bool __simplify_expressions(pNode expr)
{
    // Only work on epxression-nodes
    if (expr->type != EXPRESSION)
        return false;

    // Recursively work from down to up
    for (size_t i = 0; i < expr->n_children; i++)
    {
        pNode child = expr->children[i];
        if (child)
           __simplify_expressions(child);
    }

    if (expr->n_children == 1)
    {
        pNode child = expr->children[0];
        // Remove null expressions and transfer data
        if (EXPRDATA_TYPE(child) && expr->data == NULL)
        {
            // Transfer data to expr and release child
            if (child->type == NUMBER_DATA)
            {
                expr->data = malloc(sizeof(int));
                *(int *)expr->data = *(int *)child->data;
            }
            else
                expr->data = strdup(child->data);
            expr->type = child->type;
            expr->n_children = 0;

            node_finalize(child);
        }
        // or simplify unary minus nodes
        else if (child->type == NUMBER_DATA && *(char *)expr->data == '-')
        {
            expr->data = realloc(expr->data, sizeof(int));

            // Transfer data to expr and release child
            expr->type = child->type;
            expr->n_children = 0;
            *(int *)expr->data = -1 * *(int *)child->data;

            node_finalize(child);
        }
    }
    // Simplify const expr with operands
    else if (expr->n_children == 2 && expr->data)
    {
        // Sanity check
        pNode ch1 = expr->children[0];
        pNode ch2 = expr->children[1];
        if (ch1->type != NUMBER_DATA || ch2->type != NUMBER_DATA)
            return false;

        // Calculate new value
        int value;
        char op = *(char *)expr->data;
        switch (op)
        {
        case '+': value = TOINT(ch1) + TOINT(ch2); break;
        case '-': value = TOINT(ch1) - TOINT(ch2); break;
        case '*': value = TOINT(ch1) * TOINT(ch2); break;
        case '/': value = TOINT(ch1) / TOINT(ch2); break;
        default:  printf("Something went wrong in simplify expr\n");
                  exit(1);
        }

        // Update and transfer
        expr->data = realloc(expr->data, sizeof(int));
        expr->type = NUMBER_DATA;
        expr->n_children = 0;
        *(int *)expr->data = value;

        node_finalize(ch1);
        node_finalize(ch2);
    }


    return true;
}


void node_simplify(pNode simplified)
{
    // For each child
    for (size_t n = 0; n < simplified->n_children; n++)
    {
        // If valid
        pNode child = simplified->children[n];
        if (!child)
            continue;
        
        // Check each simplify-stage
        __simplify_single_node(&child, simplified, n);
        __simplify_expressions(child);
        __simplify_lists(child);

        // Recursively check next stage
        node_simplify(child);
    }
}
