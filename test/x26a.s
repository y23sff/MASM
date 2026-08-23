.section .text
#include "x26a_inc.s"
#include "x26a_inc.s"
#include "./x26a_inc2.s"
#ifdef NOPE
this line is garbage and must be skipped !!!
#endif
#ifndef NOPE
#extern plus1
#export _start
#endif
_start:
    mov r1, 13
    call plus1
    mov r6, r0
    mov r1, 12
    call triple
    add r0, r6
    mov r6, r0
    mov r1, 10
    call double_it
    add r0, r6
    mov r1, r0
    mov r0, 60
    syscall
