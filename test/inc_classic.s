.section .text
.globl _start
#include "inc_body.s"
_start:
    mov r1, 5
    call t2
    mov r1, r0
    mov r0, 60
    syscall
