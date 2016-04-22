#ifndef STACK_IMPL_H
#define STACK_IMPL_H

/*******************************************************************************
*       Includes
*******************************************************************************/

#include "node.h"

/*******************************************************************************
*       Types
*******************************************************************************/

typedef struct {
    size_t size;
    size_t n;
    pNode *s;
} Stack;

typedef Stack* pStack;


/*******************************************************************************
*       Functions
*******************************************************************************/

void stack_init(pStack stack);
void stack_destroy(pStack stack);
void stack_push(pStack stack, pNode node);
pNode stack_pop(pStack stack);

#endif
