.section .rodata
fmt:
    string "masm libc test: %d\n"
.section .text
#extern printf
#export main
main:
    ldr r1, fmt
    mov r2, 42
    mov r0, 0
    call printf
    mov r0, 42
    ret
