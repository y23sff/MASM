.section .text
#ifdef ARCH_X86_64
#export _start
#else
int 3
#endif
#ifndef ARCH_X86_64
int 3
#else
nop
#endif
_start:
    mov r1, 7
    mov r0, 60
    syscall
