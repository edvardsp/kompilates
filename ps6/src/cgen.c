
/*******************************************************************************
*       Includes
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cgen.h"
#include "ir.h"
#include "node.h"
#include "nodetypes.h"

/*******************************************************************************
*       Macros
*******************************************************************************/

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define ALIGN16(x) (((x)&1) ? 8*(x+1) : 8*(x))


#define ASM0(op)       puts("\t" #op)
#define ASM1(op, a)    puts("\t" #op "  \t" #a)
#define ASM2(op, a, b) puts("\t" #op "  \t" #a ", " #b)

#define FASM1(op, a, ...)    printf("\t" #op "  \t" #a "\n", __VA_ARGS__)
#define FASM2(op, a, b, ...) printf("\t" #op "  \t" #a ", " #b "\n", __VA_ARGS__)

#define ADDR(f1, addr, f2) \
    do { \
        printf("\t%s\t", #f1); \
        addr; \
        printf(", %s\n", #f2); \
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
        printf("-%zu(%%rbp)", 8*(sym->seq+1));
        break;
    case SYM_LOCAL_VAR:
        printf("-%zu(%%rbp)", 8*(func->nparms+sym->seq+1));
        break;
    default: assert(0);
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
    puts("main:");
    puts("\t# Stack frame setup");
    ASM1(pushq, %rbp);
    ASM2(movq, %rsp, %rbp);
    puts("");

    puts("\t# Sanity check on arguments");
    ASM2(subq, $1, %rdi);
    FASM2(cmpq, $%zu, %%rdi, first->nparms);
    ASM1(jne, ABORT);
    ASM2(cmpq, $0, %rdi);
    ASM1(jz, SKIP_ARGS);
    puts("");

    puts("\t# Parse args");
    ASM2(movq, %rdi, %rcx);
    FASM2(addq, $%zu, %%rsi, 8*first->nparms);
    puts("PARSE_ARGV:");
    ASM1(pushq, %rcx);
    ASM1(pushq, %rsi);
    puts("");

    ASM2(movq, (%rsi), %rdi);
    ASM2(movq, $0, %rsi);
    ASM2(movq, $10, %rdx);
    ASM1(call, strtol);
    puts("");

    puts("\t# Now a new argument is an integer in rax");
    ASM1(popq, %rsi);
    ASM1(popq, %rcx);
    ASM1(pushq, %rax);
    ASM2(subq, $8, %rsi);
    ASM1(loop, PARSE_ARGV);
    puts("");

    puts("\t# Now the arguments are in order on stack");
    for (size_t arg = 0; arg < MIN(6, first->nparms); arg++)
        FASM1(popq, %s, record[arg]);

    puts("SKIP_ARGS:");
    FASM1(call, _%s, first->name);
    ASM1(jmp, END);
    puts("ABORT:");
    ASM2(movq, $errout, %rdi);
    ASM1(call, puts);

    puts("END:");
    ASM2(movq, %rax, %rdi);
    ASM1(call, exit);
    puts("");
}


static void cgen_expression(pSymbol func, pNode expr)
{
    switch (GET_TYPE(expr))
    {
    case EXPRESSION:
    {
        // Function call
        if (!GET_DATA(expr))
        {
            puts("\t# Function call");
            pNode iden = GET_CHILD(expr, 0), args = GET_CHILD(expr, 1);
            for (size_t i = 0; i < GET_SIZE(args); i++)
            {
                printf("\t# Arg num %zu\n", i);
                pNode arg = GET_CHILD(args, i);
                cgen_expression(func, arg);
                ASM1(pushq, %rax);
            }

            for (size_t i = 1; i <= GET_SIZE(args); i++)
                FASM1(popq, %s, record[GET_SIZE(args) - i]);
            FASM1(call, _%s, GET_ENTRY(iden)->name);
            puts("");
            return;
        }

        puts("\t# Expression evaluation");

        // Unary minus
        if (GET_SIZE(expr) == 1)
        {
            puts("\t# Unary minus");
            pNode ch = GET_CHILD(expr, 0);
            ADDR(movq, cgen_resaddr(func, GET_ENTRY(ch)), %rax);
            ASM1(negq, %rax);
            return;
        }

        // Normal operator epxression
        pNode ch0 = GET_CHILD(expr, 0), ch1 = GET_CHILD(expr, 1);

        // Left hand side
        cgen_expression(func, ch0);
        ASM1(pushq, %rax);

        // Right hand side
        cgen_expression(func, ch1);

        ASM1(popq, %r8);

        switch (*(char *)GET_DATA(expr))
        {
        case '+':
            puts("\t# Addition");
            ASM2(addq, %r8, %rax);
            break;
        case '-':
            puts("\t# Subtraction");
            ASM2(subq, %rax, %r8);
            ASM2(movq, %r8, %rax);
            break;
        case '*':
            puts("\t# Multiplication");
            ASM0(cqo);
            ASM1(imulq, %r8);
            break;
        case '/':
            puts("\t# Divition");
            ASM2(xchg, %rax, %r8);
            ASM0(cqo);
            ASM1(idivq, %r8);
            break;
        }
        puts("");
        break;
    }
    case IDENTIFIER_DATA:
        ADDR(movq, cgen_resaddr(func, GET_ENTRY(expr)), %rax);
        break;
    case NUMBER_DATA:
        FASM2(movq, $%i, %%rax, *(int *)GET_DATA(expr));
        break;
    case STRING_DATA:
        FASM2(movq, $STR%zu, %%rax, expr->index);
        break;
    default: assert(0);
    }

    puts("");
}


static void cgen_assignment(pSymbol func, pNode asgn)
{
    puts("\t# Assignment");
    pNode addr = GET_CHILD(asgn, 0), expr = GET_CHILD(asgn, 1);

    cgen_expression(func, expr);

    printf("\tmovq\t%%rax, ");
    cgen_resaddr(func, GET_ENTRY(addr));
    printf("\n");

    puts("");
}


static void cgen_print(pSymbol func, pNode prt)
{
    printf("\t# Print statement, total %zu arg(s)\n", GET_SIZE(prt));
    for (size_t i = 0; i < GET_SIZE(prt); i++)
    {
        printf("\t# Arg num %zu\n", i);
        pNode ch = GET_CHILD(prt, i);

        cgen_expression(func, ch);
        ASM2(movq, %rax, %rsi);

        switch (GET_TYPE(ch))
        {
        case STRING_DATA:
            ASM2(movq, $strout, %rdi);
            break;
        case IDENTIFIER_DATA: case EXPRESSION: case NUMBER_DATA:
            ASM2(movq, $intout, %rdi);
            break;
        default: assert(0);
        }
        ASM1(call, printf);
    }

    ASM2(movq, $newlin, %rdi);
    ASM1(call, printf);

    puts("");
}


static void cgen_return(pSymbol func, pNode curr)
{
    pNode expr = GET_CHILD(curr, 0);
    cgen_expression(func, expr);
    ASM0(leave);
    ASM0(ret);

    puts("");
}

// recursive calls back and forth
static void cgen_subtree(pSymbol func, pNode curr);

static void cgen_if(pSymbol func, pNode if_stmnt)
{
    static size_t if_count = 0;

    puts("\t# If relation");
    // Normal operator epxression
    pNode relation = GET_CHILD(if_stmnt, 0);
    pNode lhs = GET_CHILD(relation, 0), rhs = GET_CHILD(relation, 1);

    // Left hand side
    cgen_expression(func, lhs);
    ASM1(pushq, %rax);

    // Right hand side
    cgen_expression(func, rhs);

    ASM1(popq, %r10);
    ASM2(cmpq, %r10, %rax);
    switch (*(char *)GET_DATA(relation))
    {
    case '=': printf("\tjne \tELSE_%zu\n", if_count); break;
    case '<': printf("\tjle \tELSE_%zu\n", if_count); break;
    case '>': printf("\tjge \tELSE_%zu\n", if_count); break;
    default: assert(0);
    }
    puts("");

    puts("\t# Actual if block");
    cgen_subtree(func, GET_CHILD(if_stmnt, 1));
    FASM1(jmp, FI_%zu, if_count);
    puts("");

    puts("\t# Actual else block");
    printf("ELSE_%zu:\n", if_count);
    if (GET_SIZE(if_stmnt) == 3)
        cgen_subtree(func, GET_CHILD(if_stmnt, 2));

    printf("FI_%zu:\n", if_count++);
    puts("");
}


static size_t while_count = 0;
static void cgen_while(pSymbol func, pNode while_stmnt)
{

    puts("\t# While loop");
    printf("WHILE_%zu:\n", ++while_count);
    pNode relation = GET_CHILD(while_stmnt, 0);
    pNode block = GET_CHILD(while_stmnt, 1);
    pNode lhs = GET_CHILD(relation, 0), rhs = GET_CHILD(relation, 1);

    // Left hand side
    puts("\t# LHS while condition");
    cgen_expression(func, lhs);
    ASM1(pushq, %rax);

    // Right hand side
    puts("\t# RHS while condition");
    cgen_expression(func, rhs);

    puts("\t# Test while condition");
    ASM1(popq, %r10);
    ASM2(cmpq, %r10, %rax);
    switch (*(char *)GET_DATA(relation))
    {
    case '=': FASM1(jne, ENDWHILE_%zu, while_count); break;
    case '<': FASM1(jle, ENDWHILE_%zu, while_count); break;
    case '>': FASM1(jge, ENDWHILE_%zu, while_count); break;
    default: assert(0);
    }
    puts("");

    cgen_subtree(func, block);

    puts("\t# End while block");
    FASM1(jmp, WHILE_%zu, while_count);
    printf("ENDWHILE_%zu:\n", while_count++);
    puts("");
}


static void cgen_continue(void)
{
    puts("\t# Continue statement");
    printf("\tjmp \tWHILE_%zu\n", while_count);
    puts("");
}


static void cgen_subtree(pSymbol func, pNode curr)
{
    switch (GET_TYPE(curr))
    {
    case PRINT_STATEMENT:      cgen_print(func, curr);      return;
    case ASSIGNMENT_STATEMENT: cgen_assignment(func, curr); return;
    case RETURN_STATEMENT:     cgen_return(func, curr);     return;
    case IF_STATEMENT:         cgen_if(func, curr);         return;
    case WHILE_STATEMENT:      cgen_while(func, curr);      return;
    case NULL_STATEMENT:       cgen_continue();             return;
    default: {}
    }

    for (size_t i = 0; i < GET_SIZE(curr); i++)
        if (GET_CHILD(curr, i))
            cgen_subtree(func, GET_CHILD(curr, i));
}


static void cgen_functions(pSymbol sym)
{
    printf("_%s:\n", sym->name);
    puts("\t# Stack frame setup");
    ASM1(pushq, %rbp);
    ASM2(movq, %rsp, %rbp);
    puts("");

    size_t nlocals = tlhash_size(sym->locals);

    if (nlocals > 0)
    {
        puts("\t# Stack setup of params and local variables");
        printf("\t# Total %zu param(s), %zu var(s)\n", sym->nparms, nlocals-sym->nparms);
        FASM2(subq, $%zu, %%rsp, ALIGN16(nlocals));
        if (sym->nparms > 0)
        {
            puts("\t# Parameters");
            for (size_t j = 0; j < sym->nparms; j++)
                FASM2(movq, %s, -%zu(%%rbp), record[j], 8*(j+1));
        }
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
