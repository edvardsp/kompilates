#ifndef IR_H
#define IR_H

/*******************************************************************************
*       Includes
*******************************************************************************/

#include "nodetypes.h"

/*******************************************************************************
*       Pragma
*******************************************************************************/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif

/*******************************************************************************
*       Types
*******************************************************************************/

typedef struct _node {
    node_index_t type;
    void *data;
    void *entry;
    size_t n_children;
    struct _node **children;
} Node;

typedef Node* pNode;

/*******************************************************************************
*       Functions
*******************************************************************************/

pNode node_create(void);
void node_init(pNode n, node_index_t type, void *data, size_t n_children, ...);
void node_print(pNode root, int nesting);
void node_reduce(node_index_t type, void *data, size_t n_pops);
void node_finalize(pNode discard);
void node_destroy(pNode discard);
void node_simplify(pNode simplified);

/*******************************************************************************
*       Pragma
*******************************************************************************/

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif
