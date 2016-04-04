
/*******************************************************************************
*       Includes
*******************************************************************************/

#include <stdio.h>

#include "cgen.h"
#include "ir.h"

/*******************************************************************************
*       Macros
*******************************************************************************/

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/*******************************************************************************
*       Globals
*******************************************************************************/

static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

/*******************************************************************************
*       Functions
*******************************************************************************/

static void cgen_stringtable(void)
{
    /* These can be used to emit numbers, strings and a run-time
     * error msg. from main
     */
    puts(".section .rodata");
    puts("intout: .string \"%ld \"");
    puts("strout: .string \"%s \"");
    puts("errout: .string \"Wrong number of arguments\"");

    /* TODO:  handle the strings from the program */

}


static void cgen_main(pSymbol first)
{
    puts(".globl main");
    puts(".section .text");
    puts("main:");
    puts("\tpushq %rbp");
    puts("\tmovq %rsp, %rbp");

    puts("\tsubq $1, %rdi");
    printf("\tcmpq\t$%zu,%%rdi\n", first->nparms);
    puts("\tjne ABORT");
    puts("\tcmpq $0, %rdi");
    puts("\tjz SKIP_ARGS");

    puts("\tmovq %rdi, %rcx");
    printf("\taddq $%zu, %%rsi\n", 8*first->nparms);
    puts("PARSE_ARGV:");
    puts("\tpushq %rcx");
    puts("\tpushq %rsi");

    puts("\tmovq (%rsi), %rdi");
    puts("\tmovq $0, %rsi");
    puts("\tmovq $10, %rdx");
    puts("\tcall strtol");

    /*  Now a new argument is an integer in rax */
    puts("\tpopq %rsi");
    puts("\tpopq %rcx");
    puts("\tpushq %rax");
    puts("\tsubq $8, %rsi");
    puts("\tloop PARSE_ARGV");

    /* Now the arguments are in order on stack */
    for (size_t arg = 0; arg < MIN(6, first->nparms); arg++)
        printf("\tpopq\t%s\n", record[arg]);

    puts("SKIP_ARGS:");
    printf("\tcall\t_%s\n", first->name);
    puts("\tjmp END");
    puts("ABORT:");
    puts("\tmovq $errout, %rdi");
    puts("\tcall puts");

    puts("END:");
    puts("\tmovq %rax, %rdi");
    puts("\tcall exit");
}


void cgen_program(void)
{
    cgen_stringtable();

    /* Put some dummy stuff to keep the skeleton from crashing */
    puts(".globl main");
    puts(".section .text");
    puts("main:");
    puts("\tmovq $0, %rax");
    puts("\tcall exit");
}
