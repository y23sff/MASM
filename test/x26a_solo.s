.section .text
#export _start
#include "x26a_inc.s"
#include "./x26a_inc2.s"
_start:
    mov r1, 12
    call triple
    mov r1, r0
    mov r0, 60
    syscall
