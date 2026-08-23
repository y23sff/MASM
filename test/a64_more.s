.section .text
.globl _start
_start:
    mov r0, 4660
    ror r0, 8
    rol r0, 4
    shl r1, r0
    sar r2, 3
    cmp r0, r1
    sete r3
    cmove r4, r1
    ret
