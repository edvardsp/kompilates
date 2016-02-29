
/*******************************************************************************
*       Includes
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "ir.h"
#include "stack.h"

/*******************************************************************************
*       Defines
*******************************************************************************/

#define BLOCK_SIZE 512

/*******************************************************************************
*       Functions
*******************************************************************************/

void stack_init(pStack stk)
{
    stk->size = BLOCK_SIZE;
    stk->n = 0;
    stk->s = (pNode *)calloc(BLOCK_SIZE, sizeof(pNode));
    if (!stk->s)
    {
        fprintf(stderr, "calloc of stack_init->s failed\n");
        exit(1);
    }
}

void stack_destroy(pStack stk)
{
    free(stk->s);
}

void stack_push(pStack stk, pNode node)
{
    if (stk->n >= stk->size)
    {
        printf("Stack resized\n");
        stk->size += BLOCK_SIZE;
        stk->s = (pNode *)realloc(stk->s, stk->size);
        if (!stk->s)
        {
            fprintf(stderr, "realloc of stack failed\n");
            exit(1);
        }
    }
    stk->s[stk->n++] = node;
}

pNode stack_pop(pStack stk)
{
    if (stk->n == 0)
    {
        fprintf(stderr, "Call for stack pop with n=0\n");
        exit(1);
    }
    return stk->s[--stk->n];
}
