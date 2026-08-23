.section .text
.globl _start
_start:
    mov r0, 0
    mov r1, 3
loop:
    cmp r0, r1
    je done
    add r0, 1
    b loop
done:
    cmp r0, r1
    jne bad
    mov r0, 0
doit:
    add r0, 1
    cmp r0, r1
    jne doit
    cmp r0, r1
    jne bad
    ret
bad:
    mov r2, 0
    div r1, r2
