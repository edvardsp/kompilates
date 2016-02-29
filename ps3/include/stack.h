#ifndef STACK_IMPL_H
#define STACK_IMPL_H

#include "ir.h"

typedef struct {
    size_t size;
    size_t n;
    pNode *s;
} Stack;

typedef Stack* pStack;

void stack_init(pStack stack);
void stack_destroy(pStack stack);
void stack_push(pStack stack, pNode node);
pNode stack_pop(pStack stack);

#endif
