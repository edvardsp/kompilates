#ifndef NODE_H
#define NODE_H

/*******************************************************************************
*       Includes
*******************************************************************************/

#include <stddef.h> // for size_t

#include "nodetypes.h"

/*******************************************************************************
*       Types
*******************************************************************************/

typedef struct _node {
    node_index_t type;
    void *data;
    size_t index;
    void *entry;
    size_t n_children;
    struct _node **children;
} Node, *pNode;

/*******************************************************************************
*       Macros
*******************************************************************************/

#define GET_CHILD(node, i) ((node)->children[(i)])
#define GET_CHILDS(node)   ((node)->children)
#define GET_ENTRY(node)    ((pSymbol)(node)->entry)
#define GET_TYPE(node)     ((node)->type)
#define GET_SIZE(node)     ((node)->n_children)
#define GET_DATA(node)     ((node)->data)
#define GET_IND(node)      ((node)->index)

/*******************************************************************************
*       Functions
*******************************************************************************/

pNode node_create  (void);
void  node_init    (pNode n, node_index_t type, void *data, size_t n_children, ...);
void  node_print   (pNode root, int nesting);
void  node_reduce  (node_index_t type, void *data, size_t n_pops);
void  node_finalize(pNode discard);
void  node_destroy (pNode discard);
void  node_simplify(pNode simplified);

#endif
