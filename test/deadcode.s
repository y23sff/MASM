.section .text
#export _start
_start:
    mov r1, 7
    jmp over
    mov r1, 99
    add r1, 55
    sub r1, 13
over:
    mov r0, 60
    syscall
