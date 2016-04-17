.section .rodata
    strfmt: .string "#%i: %llu\n"

.globl main
.section .text
main:
    pushq %rbp
    movq %rsp, %rbp

    movq $1, %r13 # number
    mov  $0, %r14  # fib prev
    mov  $1, %r15  # fib num

fib:
    mov  %r15, %rdx
    movq %r13, %rsi
    mov  $strfmt, %rdi
    call printf

    mov %r15, %rax
    add %r14, %r15
    mov %rax, %r14

    addq $1, %r13
    cmpq $51, %r13
    jne  fib

    movq $0, %rax
    call exit
