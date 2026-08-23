.section .text
.globl _start
_start:
    mov qword r0, 1
    mov qword r1, 1
    ldr qword r2, msg
    mov qword r3, 14
    syscall
    mov qword r0, 60
    xor r1, r1
    syscall

.section .rodata
msg:
    string "Hello, World!\n"
