#ifndef NODETYPES_H
#define NODETYPES_H

/*******************************************************************************
*       Types
*******************************************************************************/

typedef enum {
    PROGRAM,
    GLOBAL_LIST,
    GLOBAL,
    STATEMENT_LIST,
    PRINT_LIST,
    EXPRESSION_LIST,
    VARIABLE_LIST,
    ARGUMENT_LIST,
    PARAMETER_LIST,
    DECLARATION_LIST,
    FUNCTION,
    STATEMENT,
    BLOCK,
    ASSIGNMENT_STATEMENT,
    RETURN_STATEMENT,
    PRINT_STATEMENT,
    NULL_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    EXPRESSION,
    RELATION,
    DECLARATION,
    PRINT_ITEM,
    IDENTIFIER_DATA,
    NUMBER_DATA,
    STRING_DATA
} node_index_t;

/*******************************************************************************
*       Globals
*******************************************************************************/

extern char *node_string[26];

#endif
