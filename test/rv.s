.section .text
.globl _start
_start:
    mov r0, 6
    mov r1, 7
    mul r0, r1
    cmp r0, r1
    je eq
    mov r0, 0
eq:
    push r0
    pop r1
    b end
    nop
end:
    ret
