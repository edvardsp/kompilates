#ifndef IR_H
#define IR_H

#include <stdint.h>

#include "nodetypes.h"

typedef struct _node {
    node_index_t type;
    void *data;
    void *entry;
    uint64_t n_children;
    struct _node **children;
} node_t;

node_t* node_create(void);
void node_init(node_t *n, node_index_t type, void *data, uint64_t n_children, ...);
void node_print(node_t *root, int nesting);
void node_reduce(node_index_t type, void *data, int n_pops);
void node_finalize(node_t *discard);
void destroy_subtree(node_t *discard);

#endif
