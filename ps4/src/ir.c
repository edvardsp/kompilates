
/*******************************************************************************
*       Includes
*******************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ir.h"
#include "node.h"
#include "tlhash.h"

/*******************************************************************************
*       Defines
*******************************************************************************/

#define INSERT_SYM(table, key, sym) tlhash_insert((table), (key), strlen(key)+1, (sym))
#define LOOKUP_SYM(table, key, sym) tlhash_lookup((table), (key), strlen(key)+1, (void **)(sym))
#define GET_SCP(scope) (scope)->scp[(scope)->lvl]

/*******************************************************************************
*       Types
*******************************************************************************/

typedef struct {
    size_t lvl;
    size_t curr_seq;
    pTlhash *scp;
} Scopes, *pScopes;

/*******************************************************************************
*       Globals
*******************************************************************************/

static pTlhash global_names;
static char **string_list;
static size_t stringc = 0;
//static size_t n_string_list = 8;

/*******************************************************************************
*       Static Functions
*******************************************************************************/

static void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (size_t)rand() % (sizeof(charset) - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}


static void traverse_node(pSymbol func, pNode node, pScopes scps)
{
    assert(node != NULL);

    switch (GET_TYPE(node))
    {
    case DECLARATION:
    {
        // Create symbol and add to correct symbol tables
        pNode decl = GET_CHILD(node, 0);
        for (size_t i = 0; i < GET_SIZE(decl); i++)
        {
            pSymbol sym = malloc(sizeof(Symbol));
            *sym = (Symbol){
                .name = GET_DATA(GET_CHILD(decl, i)),
                .type = SYM_LOCAL_VAR,
                .node = node,
                .seq = scps->curr_seq++,
                .nparms = 0,
                .locals = NULL
            };

            // TODO: rand_str MIGHT produce a current key in use
            char key[10];
            rand_str(key, 9);
            INSERT_SYM(func->locals, key, sym);
            INSERT_SYM(GET_SCP(scps), sym->name, sym);
        }

        return;
    }
    case IDENTIFIER_DATA:
    {
        pSymbol sym;
        char *name = GET_DATA(node);

        // Lookup name in order:
        // 1. scopes, descending order
        // 2. function locales
        // 3. globale locales
        for (size_t i = 0; i <= scps->lvl; i++)
        {
            LOOKUP_SYM(scps->scp[scps->lvl - i], name, &sym);
            if (sym) goto sym_found;
        }
        LOOKUP_SYM(func->locals, name, &sym); if (sym) goto sym_found;
        LOOKUP_SYM(global_names, name, &sym); if (sym) goto sym_found;
        fprintf(stderr, "Error: Symbol `%s` doesn't exist\n", name);
        exit(1);

    sym_found:
        node->entry = sym;
        return;
    }
    case STRING_DATA:
        // Move string from node to string_list, and assign index to node
        string_list[stringc] = GET_DATA(node);
        GET_DATA(node) = NULL;
        GET_IND(node) = stringc++;
        string_list = realloc(string_list, sizeof(char *) * (stringc + 1));
        return;

    default: {}
    }

    // If block setup next scope
    if (GET_TYPE(node) == BLOCK)
    {
        size_t lvl = ++scps->lvl;
        scps->scp = realloc(scps->scp, sizeof(pTlhash) * (lvl + 1));
        GET_SCP(scps) = malloc(sizeof(Tlhash));
        tlhash_init(GET_SCP(scps), 16);
    }

    // Traverse tree pre-order
    for (size_t i = 0; i < GET_SIZE(node); i++)
        traverse_node(func, GET_CHILD(node, i), scps);

    // Cleanup after block
    if (GET_TYPE(node) == BLOCK)
    {
        tlhash_finalize(GET_SCP(scps));
        free(GET_SCP(scps));
        scps->lvl--;

    }

}

/*******************************************************************************
*       Functions
*******************************************************************************/

void ir_init(void)
{
    string_list = malloc(sizeof(char *));
    global_names = malloc(sizeof(Tlhash));
    tlhash_init(global_names, 16);
}


void ir_destroy(void)
{
    size_t n_globals = tlhash_size(global_names);
    pSymbol *global_list = malloc(sizeof(pSymbol) * n_globals);
    tlhash_values(global_names, (void **)global_list);

    // For all global symbols
    for (size_t n = 0; n < n_globals; n++)
    {
        // If function
        if (global_list[n]->type == SYM_FUNCTION)
        {
            pTlhash local_names = global_list[n]->locals;
            size_t n_locals = tlhash_size(local_names);
            pSymbol *local_list = malloc(sizeof(pSymbol) * n_locals);
            tlhash_values(local_names, (void **)local_list);

            // Release all symbols in function locale
            for (size_t m = 0; m < n_locals; m++)
                free(local_list[m]);

            tlhash_finalize(local_names);
            free(local_names);
            free(local_list);
        }

        // Release the global symbol
        free(global_list[n]);
    }

    // Release global symbol table and hash
    tlhash_finalize(global_names);
    free(global_names);
    free(global_list);

    for (size_t i = 0; i < stringc; i++)
        free(string_list[i]);
    free(string_list);
}


void ir_find_globals(pNode root)
{
    assert(root != NULL);

    // For all in global list
    size_t n_func = 0;
    pNode global_list = root->children[0];
    for (size_t i = 0; i < global_list->n_children; i++)
    {
        pNode entry = GET_CHILD(global_list, i);

        // If decleration
        if (GET_TYPE(entry) == DECLARATION)
        {
            // For all in declaration list
            pNode list = GET_CHILD(entry, 0);
            for (size_t j = 0; j < GET_SIZE(list); j++)
            {
                // Add to symbol table
                pSymbol sym = malloc(sizeof(Symbol));
                *sym = (Symbol){
                    .name = GET_DATA(GET_CHILD(list, j)),
                    .type = SYM_GLOBAL_VAR,
                    .node = entry,
                    .seq = j,
                    .nparms = 0,
                    .locals = NULL
                };

                INSERT_SYM(global_names, sym->name, sym);
            }
        }
        // If function
        else if (GET_TYPE(entry) == FUNCTION)
        {
            // Init symbol struct
            pNode ident = GET_CHILD(entry, 0);
            size_t nparms = GET_CHILD(entry, 1)
                          ? GET_SIZE(GET_CHILD(entry, 1))
                          : 0;
            pSymbol sym = malloc(sizeof(Symbol));
            pTlhash locals = malloc(sizeof(Tlhash));
            tlhash_init(locals, 16);

            *sym = (Symbol){
                .name = GET_DATA(ident),
                .type = SYM_FUNCTION,
                .node = entry,
                .seq = n_func++,
                .nparms = nparms,
                .locals = locals
            };

            // For all params
            for (size_t k = 0; k < nparms; k++)
            {
                // Add to locals, and sequence
                pNode param = GET_CHILD(GET_CHILD(entry, 1), k);
                pSymbol psym = malloc(sizeof(Symbol));

                *psym = (Symbol) {
                    .name = GET_DATA(param),
                    .type = SYM_PARAMETER,
                    .node = entry,
                    .seq = k,
                    .nparms = 0,
                    .locals = NULL
                };

                INSERT_SYM(locals, psym->name, psym);
            }

            INSERT_SYM(global_names, sym->name, sym);
        }
    }
}


void ir_bind_names(pSymbol func, pNode root)
{
    assert(func != NULL);
    assert(root != NULL);

    // Setup scoping
    Scopes scopes = {
        .lvl = 0,
        .curr_seq = 0,
        .scp = malloc(sizeof(pTlhash))
    };
    *scopes.scp = malloc(sizeof(Tlhash));
    tlhash_init(*scopes.scp, 16);

    // Skip identifier and parameters
    pNode block = GET_CHILD(root, 2);
    for (size_t i = 0; i < GET_SIZE(block); i++)
        traverse_node(func, GET_CHILD(block, i), &scopes);

    // Free scoping
    tlhash_finalize(*scopes.scp);
    free(*scopes.scp);
    free(scopes.scp);
}


void ir_print_symbols(void)
{
    printf("String table:\n");
    for (size_t s = 0; s < stringc; s++)
        printf("%zu: %s\n", s, string_list[s]);
    printf("--\n");

    printf("Globals:\n");
    size_t n_globals = tlhash_size(global_names);
    pSymbol *global_list = malloc(sizeof(pSymbol) * n_globals);
    tlhash_values(global_names, (void **)global_list);

    for (size_t g = 0; g < n_globals; g++)
    {
        if (global_list[g]->type == SYM_FUNCTION)
        {
            printf(
                "%s: function %zu:\n",
                global_list[g]->name,
                global_list[g]->seq
            );
            if (global_list[g]->locals != NULL)
            {
                size_t localsize = tlhash_size(global_list[g]->locals);
                printf(
                    "\t%zu local variables, %zu are parameters:\n",
                    localsize,
                    global_list[g]->nparms
                );
                pSymbol *locals = malloc(sizeof(pSymbol) * localsize);
                tlhash_values(global_list[g]->locals, (void **)locals);
                for (size_t i = 0; i < localsize; i++)
                {
                    printf("\t%s: ", locals[i]->name);

                    if (locals[i]->type == SYM_PARAMETER)
                        printf("parameter %zu\n", locals[i]->seq);
                    else if (locals[i]->type == SYM_LOCAL_VAR)
                        printf("local var %zu\n", locals[i]->seq);
                }
                free(locals);
            }
        }
        else if (global_list[g]->type == SYM_GLOBAL_VAR)
        {
            printf("%s: global variable\n", global_list[g]->name);
        }
    }
    free(global_list);
    printf("--\n");
}


void ir_print_bindings(pNode root)
{
    if (root == NULL)
        return;

    pSymbol entry = root->entry;
    if (entry != NULL)
    {
        switch (entry->type)
        {
        case SYM_GLOBAL_VAR:
            printf("Linked global var '%s'\n", entry->name);
            break;
        case SYM_FUNCTION:
            printf("Linked function %zu ('%s')\n", entry->seq, entry->name);
            break;
        case SYM_PARAMETER:
            printf("Linked parameter %zu ('%s')\n", entry->seq, entry->name);
            break;
        case SYM_LOCAL_VAR:
            printf("Linked local var %zu ('%s')\n", entry->seq, entry->name);
            break;
        }
    } else if (root->type == STRING_DATA) {
        if (GET_IND(root) < stringc)
            printf("Linked string %zu\n", GET_IND(root));
        else
            printf("(Not an indexed string)\n");
    }
    for (size_t c = 0; c < root->n_children; c++)
        ir_print_bindings(root->children[c]);
}


void ir_obtain_all(pNode root)
{
    assert(root != NULL);

    // Iterate over all global symbols, resolve uses of variables:
    // Obtain all global names
    size_t n_globals = tlhash_size(global_names);
    pSymbol *global_list = malloc(sizeof(pSymbol) * n_globals);
    tlhash_values(global_names, (void **)global_list);

    // Call bind_names on all those which are functions
    for (size_t i = 0; i < n_globals; i++)
    {
        pSymbol sym = global_list[i];
        if (sym->type == SYM_FUNCTION)
            ir_bind_names(sym, sym->node);
    }
    free(global_list);
}


void ir_print_final(pNode root)
{
    assert(root != NULL);

    // Print the final state of the symbol table(s)
    ir_print_symbols();
    printf("Bindings:\n");
    ir_print_bindings(root);
}
