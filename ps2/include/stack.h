#ifndef __STACK_H
#define __STACK_H

#include "ir.h"

typedef struct {
    int size;
    int n;
    node_t **s;
} stack_t;

void stack_init(stack_t *stack);
void stack_destroy(stack_t *stack);
void stack_push(stack_t *stack, node_t *node);
node_t* stack_pop(stack_t *stack);


#endif