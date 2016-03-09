
/*******************************************************************************
*       Includes
*******************************************************************************/

#include "nodetypes.h"

/*******************************************************************************
*       Defines
*******************************************************************************/

#define STRINGIFY(x) #x

/*******************************************************************************
*       Globals
*******************************************************************************/

char *node_string[26] = {
    STRINGIFY(PROGRAM),
    STRINGIFY(GLOBAL_LIST),
    STRINGIFY(GLOBAL),
    STRINGIFY(STATEMENT_LIST),
    STRINGIFY(PRINT_LIST),
    STRINGIFY(EXPRESSION_LIST),
    STRINGIFY(VARIABLE_LIST),
    STRINGIFY(ARGUMENT_LIST),
    STRINGIFY(PARAMETER_LIST),
    STRINGIFY(DECLARATION_LIST),
    STRINGIFY(FUNCTION),
    STRINGIFY(STATEMENT),
    STRINGIFY(BLOCK),
    STRINGIFY(ASSIGNMENT_STATEMENT),
    STRINGIFY(RETURN_STATEMENT),
    STRINGIFY(PRINT_STATEMENT),
    STRINGIFY(NULL_STATEMENT),
    STRINGIFY(IF_STATEMENT),
    STRINGIFY(WHILE_STATEMENT),
    STRINGIFY(EXPRESSION),
    STRINGIFY(RELATION),
    STRINGIFY(DECLARATION),
    STRINGIFY(PRINT_ITEM),
    STRINGIFY(IDENTIFIER_DATA),
    STRINGIFY(NUMBER_DATA),
    STRINGIFY(STRING_DATA)
};

#undef STRINGIFY
