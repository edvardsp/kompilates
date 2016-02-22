#ifndef IR_H
#define IR_H

#include "nodetypes.h"

typedef struct _node {
    node_index_t type;
    void *data;
    void *entry;
    int n_children;
    struct _node **children;
} Node;

typedef Node* pNode;

pNode node_create(void);
void node_init(pNode n, node_index_t type, void *data, int n_children, ...);
void node_print(pNode root, int nesting);
void node_reduce(node_index_t type, void *data, int n_pops);
void node_finalize(pNode discard);
void node_destroy(pNode discard);
void node_simplify(pNode *simplified, pNode parent);

#endif
