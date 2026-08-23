.section .text
.globl _start
_start:
    shl r0, r1
    shr r2, 3
    ror r3, 1
    rol r4, 1
    cmp r0, r1
    sete r2
    ret
