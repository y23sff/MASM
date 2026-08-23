.section .text
.globl _start
_start:
    mov r0, 4          ; sys_write (i386): eax=4
    mov r6, 1          ; ebx = fd 1
    ldr r4, msg        ; ecx = buf
    mov r3, 15         ; edx = len
    int 0x80
    mov r0, 1          ; sys_exit
    mov r6, 0
    int 0x80

.section .rodata
msg:
    string "Hello, 32bit!\n"
