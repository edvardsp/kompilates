
#include <stdlib.h>
#include <stdio.h>

#include "ir.h"
#include "stack.h"

#define BLOCK_SIZE 512

void stack_init(stack_t *stk)
{
    stk->size = BLOCK_SIZE;
    stk->n = 0;
    stk->s = (node_t **)malloc(BLOCK_SIZE * sizeof(node_t *));
    if (!stk->s)
    {
        fprintf(stderr, "malloc of stack_init->s failed\n");
        exit(1);
    }
}

void stack_destroy(stack_t *stk)
{
    free(stk->s);
}

void stack_push(stack_t *stk, node_t *node)
{
    if (stk->n >= stk->size)
    {
        printf("Stack resized\n");
        stk->size += BLOCK_SIZE;
        stk->s = (node_t **)realloc(stk->s, stk->size);
        if (!stk->s)
        {
            fprintf(stderr, "realloc of stack failed\n");
            exit(1);
        }
    }
    stk->s[stk->n++] = node;
}

node_t* stack_pop(stack_t *stk)
{
    if (stk->n == 0)
    {
        fprintf(stderr, "Call for stack pop with n=0\n");
        exit(1);
    }
    return stk->s[--stk->n];
}
