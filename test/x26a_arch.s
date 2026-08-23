.section .text
#export _start
_start:
#ifdef ARCH_X86_64
    mov r1, 42
    mov r0, 60
    syscall
#endif
#ifdef ARCH_ARM64
    mov r0, 43
    ret
#endif
#ifdef FORMAT_COFF
    nop
#endif
