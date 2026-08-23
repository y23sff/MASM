.section .text
#export _start
_start:
    mov tmp, 42
    mov r1, tmp
    mov r0, 60
    syscall
