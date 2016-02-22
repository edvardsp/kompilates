#ifndef __STACK_H
#define __STACK_H

#include "ir.h"

typedef struct {
    int size;
    int n;
    pNode *s;
} Stack;

typedef Stack* pStack;

void stack_init(pStack stack);
void stack_destroy(pStack stack);
void stack_push(pStack stack, pNode node);
pNode stack_pop(pStack stack);


#endif