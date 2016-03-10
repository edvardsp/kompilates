
#include <stdlib.h>
#include <stdio.h>

#include "ir.h"
#include "node.h"
#include "tlhash.h"

pTlhash global_names;
char **string_list;
size_t n_string_list = 8, stringc = 0;


void ir_find_globals(void)
{
    global_names = malloc(sizeof(Tlhash));
    tlhash_init(global_names, 32);
}


void ir_bind_names(pSymbol function, pNode root)
{
    root = NULL;
    function = NULL;
}


void ir_symtab_destroy(void)
{
    tlhash_finalize(global_names);
    free(global_names);
}


void ir_print_symbols(void)
{
    printf("String table:\n");
    for (size_t s = 0; s < stringc; s++)
        printf("%zu: %s\n", s, string_list[s]);
    printf("-- \n");

    printf("Globals:\n");
    size_t n_globals = tlhash_size(global_names);
    pSymbol *global_list = malloc(sizeof(pSymbol) * n_globals);
    tlhash_values(global_names, (void **)&global_list);

    for (size_t g = 0; g < n_globals; g++)
    {
        switch (global_list[g]->type)
        {
        case SYM_FUNCTION:
            printf("%s: function %zu:\n", global_list[g]->name, global_list[g]->seq);
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

                    switch (locals[i]->type)
                    {
                    case SYM_PARAMETER:
                        printf("parameter %zu\n", locals[i]->seq);
                        break;
                    case SYM_LOCAL_VAR:
                        printf("local var %zu\n", locals[i]->seq);
                        break;
                    default: {}
                    }
                }
                free(locals);
            }
            break;
        case SYM_GLOBAL_VAR:
            printf("%s: global variable\n", global_list[g]->name);
            break;
        default: {}
        }
    }
    free(global_list);
    printf("-- \n");
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
        size_t string_index = *(size_t *)(root->data);
        if (string_index < stringc)
            printf("Linked string %zu\n", *(size_t *)(root->data));
        else
            printf("(Not an indexed string)\n");
    }
    for (size_t c = 0; c < root->n_children; c++)
        ir_print_bindings(root->children[c]);
}


void ir_obtain_all(void)
{
    // Iterate over all global symbols, resolve uses of variables:
    // Obtain all global names
    size_t n_globals = tlhash_size(global_names);
    pSymbol *global_list = malloc(sizeof(pSymbol) * n_globals);
    tlhash_values(global_names, (void **)&global_list);

    // Call bind_names on all those which are functions
    for (size_t i = 0; i < n_globals; i++)
        if (global_list[i]->type == SYM_FUNCTION)
            ir_bind_names(global_list[i], global_list[i]->node);
    free(global_list);
}


void ir_print_final(pNode root)
{
    // Print the final state of the symbol table(s)
    ir_print_symbols();
    printf("Bindings:\n");
    ir_print_bindings(root);
}
