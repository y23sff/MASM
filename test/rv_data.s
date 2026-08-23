.section .text
.globl _start
_start:
    ldr r0, msg
    load r1, msg
    str r2, buf
    cmp r0, 42
    ret
.section .data
msg:
    string "ab"
buf:
    resb 8
