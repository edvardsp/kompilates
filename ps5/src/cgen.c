
/*******************************************************************************
*       Includes
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgen.h"
#include "ir.h"
#include "node.h"
#include "nodetypes.h"

/*******************************************************************************
*       Macros
*******************************************************************************/

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define ASM0(op)     puts("\t" #op)
#define ASM1(op,a)   puts("\t" #op "\t" #a)
#define ASM2(op,a,b) puts("\t" #op "\t" #a ", " #b)
#define ADDR(f1, addr, f2) \
    do { \
        printf(f1); \
        addr; \
        printf(f2); \
    } while (0);

/*******************************************************************************
*       Globals
*******************************************************************************/

static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

/*******************************************************************************
*       Functions
*******************************************************************************/

static void cgen_resaddr(pSymbol func, pSymbol sym)
{
    switch (sym->type)
    {
    case SYM_GLOBAL_VAR:
        printf("_%s", sym->name);
        break;
    case SYM_PARAMETER:
        printf("-%zu(%%rbp)", 8*sym->seq);
        break;
    case SYM_LOCAL_VAR:
        printf("-%zu(%%rbp)", 8*(func->nparms+sym->seq));
        break;
    default: {}
    }
}

static void cgen_stringtable(void)
{
    /* These can be used to emit numbers, strings and a run-time
     * error msg. from main
     */
    puts(".section .rodata");
    puts("intout: .string \"%ld \"");
    puts("strout: .string \"%s \"");
    puts("errout: .string \"Wrong number of arguments\"");
    puts("newlin: .string \"\\n\"");

    for (size_t i = 0; i < ir.stringc; i++)
        printf("STR%zu:\t.string %s\n", i, ir.string_list[i]);
    puts("");
}


static void cgen_globalvars(size_t n_globals, pSymbol *global_list)
{
    puts(".section .data");

    for (size_t i = 0; i < n_globals; i++)
        if (global_list[i]->type == SYM_GLOBAL_VAR)
            printf("\t_%s:\t.zero 8\n", global_list[i]->name);
    puts("");
}


static void cgen_main(pSymbol first)
{
    // Set up stack frame
    puts("main:");
    ASM1(pushq, %rbp);
    ASM2(movq, %rsp, %rbp);
    puts("");

    // Sanity check on arguments
    ASM2(subq, $1, %rdi);
    printf("\tcmpq\t$%zu,%%rdi\n", first->nparms);
    ASM1(jne, ABORT);
    ASM2(cmpq, $0, %rdi);
    ASM1(jz, SKIP_ARGS);
    puts("");

    //
    ASM2(movq, %rdi, %rcx);
    printf("\taddq $%zu, %%rsi\n", 8*first->nparms);
    ASM0(PARSE_ARGV:);
    ASM1(pushq, %rcx);
    ASM1(pushq, %rsi);
    puts("");

    ASM2(movq, (%rsi), %rdi);
    ASM2(movq, $0, %rsi);
    ASM2(movq, $10, %rdx);
    ASM1(call, strtol);
    puts("");

    /*  Now a new argument is an integer in rax */
    ASM1(popq, %rsi);
    ASM1(popq, %rcx);
    ASM1(pushq, %rax);
    ASM2(subq, $8, %rsi);
    ASM1(loop, PARSE_ARGV);
    puts("");

    /* Now the arguments are in order on stack */
    for (size_t arg = 0; arg < MIN(6, first->nparms); arg++)
        printf("\tpopq\t%s\n", record[arg]);

    ASM0(SKIP_ARGS:);
    printf("\tcall\t_%s\n", first->name);
    ASM1(jmp, END);
    ASM0(ABORT:);
    ASM2(movq, $errout, %rdi);
    ASM1(call, puts);

    ASM0(END:);
    ASM2(movq, %rax, %rdi);
    ASM1(call, exit);
    puts("");
}


static void cgen_expression(pSymbol func, pNode expr)
{
    // Function call
    if (!GET_DATA(expr))
    {
        // TODO for ps6
        //pNode iden = GET_CHILD(expr, 0), args = GET_CHILD(expr, 1);
        //for (size_t i = 0; i < GET_SIZE(args); i++)
        //{
        //    switch ()
        //}

        return;
    }

    // Unary minus
    if (GET_SIZE(expr) == 1)
    {
        pNode ch = GET_CHILD(expr, 0);
        ADDR("\tmovq ", cgen_resaddr(func, GET_ENTRY(ch)), ", %%rax\n");
        ASM1(neg, %rax);
        return;
    }

    // Normal operator epxression
    pNode ch0 = GET_CHILD(expr, 0), ch1 = GET_CHILD(expr, 1);
    // Left hand side
    if (GET_TYPE(ch0) == EXPRESSION)
    {
        cgen_expression(func, ch0);
        ASM1(pushq, %rax);
    }
    if (GET_TYPE(ch0) == IDENTIFIER_DATA)
        ADDR("\tpushq ", cgen_resaddr(func, GET_ENTRY(ch0)), "\n");

    // Right hand side
    if (GET_TYPE(ch1) == EXPRESSION)
        cgen_expression(func, ch1);
    if (GET_TYPE(ch1) == IDENTIFIER_DATA)
        ADDR("\tmovq ", cgen_resaddr(func, GET_ENTRY(ch1)), ", %%rax\n");

    ASM1(popq, %r8);

    switch (*(char *)GET_DATA(expr))
    {
    case '+':
        ASM2(addq, %r8, %rax);
        break;
    case '-':
        ASM2(subq, %rax, %r8);
        ASM2(movq, %r8, %rax);
        break;
    case '*':
        ASM0(cqo);
        ASM1(imulq, %r8);
        break;
    case '/':
        ASM2(xchg, %rax, %r8);
        ASM0(cqo);
        ASM1(idivq, %r8);
        break;
    }
    puts("");
}


static void cgen_varlist(pSymbol func, pNode varl)
{
    printf("\tsubq\t$%zu, %%rsp\n", 8 * GET_SIZE(varl));
    for (size_t j = 0; j < GET_SIZE(varl); j++)
        printf("\tmovq\t$0, -%zu(%%rbp)\n", 8*(func->nparms+j));
    if ((GET_SIZE(varl) & 1) == 1)
            ASM1(pushq, $0);
    puts("");
}


static void cgen_assignment(pSymbol func, pNode asgn)
{
    pNode ch0 = GET_CHILD(asgn, 0), ch1 = GET_CHILD(asgn, 1);
    if (GET_TYPE(ch1) == EXPRESSION)
        cgen_expression(func, ch1);
    printf("\tmovq\t");
    if (GET_TYPE(ch1) == IDENTIFIER_DATA)
        cgen_resaddr(func, GET_ENTRY(ch1));
    else
        printf("%%rax ");
    puts(", %r11");

    ADDR("\tmovq\t%%r11, ", cgen_resaddr(func, GET_ENTRY(ch0)), "\n");
    puts("");
}


static void cgen_print(pSymbol func, pNode prt)
{
    for (size_t i = 0; i < GET_SIZE(prt); i++)
    {
        pNode ch = GET_CHILD(prt, i);
        pSymbol sym = GET_ENTRY(ch);
        switch (GET_TYPE(ch))
        {
        case STRING_DATA:
            ASM2(movq, $strout, %rdi);
            printf("\tmovq\t$STR%zu, %%rsi\n", ch->index);
            ASM1(call, printf);
            break;
        case IDENTIFIER_DATA:
            ASM2(movq, $intout, %rdi);
            ADDR("\tmovq\t", cgen_resaddr(func, sym), ", %%rsi\n");
            ASM1(call, printf);
            break;
        case EXPRESSION:
            cgen_expression(func, ch);
            ASM2(movq, $intout, %rdi);
            ASM2(movq, %rax, %rsi);
            ASM1(call, printf);
            break;
        case NUMBER_DATA:
            ASM2(movq, $intout, %rdi);
            printf("\tmovq\t$%i, %%rsi\n", *(int *)GET_DATA(ch));
            ASM1(call, printf);
            break;
        default: {}
        }
    }

    ASM2(movq, $newlin, %rdi);
    ASM1(call, printf);
    puts("");
}


static void cgen_returnstmn(pSymbol func, pNode curr)
{
    pNode ch = GET_CHILD(curr, 0);
    switch (GET_TYPE(ch))
    {
    case IDENTIFIER_DATA:
        ADDR("\tmovq\t", cgen_resaddr(func, GET_ENTRY(ch)), ", %%rax\n");
        break;
    case NUMBER_DATA:
        printf("\tmovq\t$%i, %%rax\n", *(int *)GET_DATA(ch));
        break;
    case EXPRESSION:
        cgen_expression(func, curr);
    default: {}
    }
    ASM0(leave);
    ASM0(ret);
    puts("");
}


static void cgen_subtree(pSymbol func, pNode curr)
{
    switch (GET_TYPE(curr))
    {
    case VARIABLE_LIST:        cgen_varlist(func, curr); return;
    case PRINT_STATEMENT:      cgen_print(func, curr); return;
    case ASSIGNMENT_STATEMENT: cgen_assignment(func, curr); return;
    case RETURN_STATEMENT:     cgen_returnstmn(func, curr); return;
    default: {}
    }

    for (size_t i = 0; i < GET_SIZE(curr); i++)
        if (GET_CHILD(curr, i))
            cgen_subtree(func, GET_CHILD(curr, i));
}


static void cgen_functions(pSymbol sym)
{
    printf("_%s:\n", sym->name);
    ASM1(pushq, %rbp);
    ASM2(movq, %rsp, %rbp);
    puts("");

    if (sym->nparms > 0)
    {
        printf("\tsubq\t$%zu, %%rsp\n", 8 * sym->nparms);
        for (size_t j = 0; j < sym->nparms; j++)
            printf("\tmovq\t%s, -%zu(%%rbp)\n", record[j], 8*j);
        if ((sym->nparms & 1) == 1)
            ASM1(pushq, $0);
        puts("");
    }
    cgen_subtree(sym, GET_CHILD(sym->node, 2));
}


void cgen_program(void)
{
    size_t n_globals = tlhash_size(ir.global_names);
    pSymbol *global_list = malloc(sizeof(pSymbol) * n_globals);
    tlhash_values(ir.global_names, (void **)global_list);

    cgen_stringtable();
    cgen_globalvars(n_globals, global_list);
    puts(".globl main");
    puts(".section .text");

    pSymbol sym_main = NULL;
    for (size_t i = 0; i < n_globals; i++)
    {
        pSymbol sym = global_list[i];
        if (sym->type != SYM_FUNCTION) continue;
        if (sym->seq == 0) sym_main = sym;
        cgen_functions(sym);
    }

    if (sym_main)
        cgen_main(sym_main);
    else
    {
        puts("Error: No main found");
        exit(1);
    }
    free(global_list);
}
