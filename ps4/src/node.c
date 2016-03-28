
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

#define TOINT(node) (*(int *)(GET_DATA(node)))


#define SINGLE_TYPE(node) \
    (GET_TYPE(node) == GLOBAL         || \
     GET_TYPE(node) == STATEMENT      || \
     GET_TYPE(node) == PRINT_ITEM     || \
     GET_TYPE(node) == ARGUMENT_LIST  || \
     GET_TYPE(node) == PARAMETER_LIST)


#define LIST_TYPE(node) \
    (GET_TYPE(node) == PRINT_LIST       || \
     GET_TYPE(node) == GLOBAL_LIST      || \
     GET_TYPE(node) == VARIABLE_LIST    || \
     GET_TYPE(node) == STATEMENT_LIST   || \
     GET_TYPE(node) == EXPRESSION_LIST  || \
     GET_TYPE(node) == PRINT_STATEMENT  || \
     GET_TYPE(node) == DECLARATION_LIST)


#define EXPRDATA_TYPE(node) \
    (GET_TYPE(node) == NUMBER_DATA  || \
     GET_TYPE(node) == IDENTIFIER_DATA)


#define STRDATA_TYPE(node) \
    (GET_TYPE(node) == RELATION     || \
     GET_TYPE(node) == EXPRESSION   || \
     GET_TYPE(node) == STRING_DATA  || \
     GET_TYPE(node) == IDENTIFIER_DATA)

/*******************************************************************************
*       Functions
*******************************************************************************/

/*******************************************************************************
*   node_create
*   |   Allocates memory for a node object
*******************************************************************************/

pNode node_create()
{
    pNode out = calloc(1, sizeof(Node));
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
        printf("%*c%s", nesting, ' ', node_string[GET_TYPE(curr)]);

        /* For identifiers, strings, expressions and numbers,
         * print the data element also
         */
        if (STRDATA_TYPE(curr))
            printf("(%s)", (char *)GET_DATA(curr));
        else if (GET_TYPE(curr) == NUMBER_DATA)
            printf("(%i)", *(int *)GET_DATA(curr));

        /* Make a new line, and traverse the node's children in the same manner */
        putchar('\n');
        for (size_t i = 0; i < GET_SIZE(curr); i++)
            node_print(GET_CHILD(curr, i), nesting + 1);
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

    GET_TYPE(nd)  = type;
    GET_DATA(nd)  = data;
    GET_IND(nd)   = 0;
    GET_ENTRY(nd) = NULL;
    GET_SIZE(nd)  = n_children;
    if (n_children > 0)
        GET_CHILDS(nd) = calloc(n_children, sizeof(pNode));
    for (size_t i = 0; i < n_children; i++)
        GET_CHILD(nd, i) = va_arg(argp, pNode);

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

    free(GET_DATA(discard));
    //free(GET_ENTRY(discard)); -- We let ir.c take care of this
    free(GET_CHILDS(discard));
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

    if (GET_CHILDS(discard))
    {
        for (size_t i = 0; i < GET_SIZE(discard); i++)
            node_destroy(GET_CHILD(discard, i));
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
        assert(GET_SIZE(*child) == 1);

        // Update children array and free memory
        GET_CHILD(parent, n) = GET_CHILD(*child, 0);
        node_finalize(*child);

        // Update the pointer
        *child = GET_CHILD(parent, n);

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
    for ( ; n < GET_SIZE(current); n++)
    {
        pNode same = GET_CHILD(current, n);
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
    size_t new_size = GET_SIZE(current) - 1 + GET_SIZE(next);
    pNode *buf1 = realloc(GET_CHILDS(current), new_size * sizeof(pNode));
    if (!buf1)
    {
        printf("Realloc during simplfy lists failed\n");
        exit(1);
    }
    GET_CHILDS(current) = buf1;

    // Extend gap of next node for possible more children
    size_t extend = GET_SIZE(next) - 1;
    if (extend > 0 && n < GET_SIZE(current) - 1)
    {
        pNode *buf2 = memmove(
            &GET_CHILD(current, n + 1 + extend),
            &GET_CHILD(current, n + 1),
            sizeof(pNode) * (GET_SIZE(current) - n - 1)
        );
        if (!buf2)
        {
            printf("Memmove during simplfy lists failed\n");
            exit(1);
        }
    }

    // Append new childs
    for (size_t i = 0; i < GET_SIZE(next); i++)
        GET_CHILD(current, n + i) = GET_CHILD(next, i);
    GET_SIZE(current) = new_size;

    // Free next node
    node_finalize(next);

    return true;
}


static bool __simplify_expressions(pNode expr)
{
    // Only work on epxression-nodes
    if (GET_TYPE(expr) != EXPRESSION)
        return false;

    // Recursively work from down to up
    for (size_t i = 0; i < GET_SIZE(expr); i++)
        if (GET_CHILD(expr, i))
           __simplify_expressions(GET_CHILD(expr, i));

    if (GET_SIZE(expr) == 1)
    {
        pNode child = GET_CHILD(expr, 0);
        // Remove null expressions and transfer data
        if (EXPRDATA_TYPE(child) && GET_DATA(expr) == NULL)
        {
            // Transfer data to expr and release child
            if (GET_TYPE(child) == NUMBER_DATA)
            {
                GET_DATA(expr) = malloc(sizeof(int));
                *(int *)GET_DATA(expr) = *(int *)GET_DATA(child);
            }
            else
                GET_DATA(expr) = strdup(GET_DATA(child));
            GET_TYPE(expr) = GET_TYPE(child);
            GET_SIZE(expr) = 0;

            node_finalize(child);
        }
        // or simplify unary minus nodes
        else if (GET_TYPE(child) == NUMBER_DATA && *(char *)GET_DATA(expr) == '-')
        {
            GET_DATA(expr) = realloc(GET_DATA(expr), sizeof(int));

            // Transfer data to expr and release child
            GET_TYPE(expr) = GET_TYPE(child);
            GET_SIZE(expr) = 0;
            *(int *)GET_DATA(expr) = -1 * *(int *)GET_DATA(child);

            node_finalize(child);
        }
    }
    // Simplify const expr with operands
    else if (GET_SIZE(expr) == 2 && GET_DATA(expr))
    {
        // Sanity check
        pNode ch1 = GET_CHILD(expr, 0);
        pNode ch2 = GET_CHILD(expr, 1);
        if (GET_TYPE(ch1) != NUMBER_DATA || GET_TYPE(ch2) != NUMBER_DATA)
            return false;

        // Calculate new value
        int value;
        char op = *(char *)GET_DATA(expr);
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
        GET_DATA(expr) = realloc(GET_DATA(expr), sizeof(int));
        GET_TYPE(expr) = NUMBER_DATA;
        GET_SIZE(expr) = 0;
        *(int *)GET_DATA(expr) = value;

        node_finalize(ch1);
        node_finalize(ch2);
    }


    return true;
}


void node_simplify(pNode simplified)
{
    // For each child
    for (size_t n = 0; n < GET_SIZE(simplified); n++)
    {
        // If valid
        pNode child = GET_CHILD(simplified, n);
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
