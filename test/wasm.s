.section .text
.globl _start
_start:
    mov r0, 6
    mov r1, 7
    mul r0, r1          ; r0 = 42
    cmp r0, r1          ; 42 vs 7
    jne skip            ; taken: must skip the div by zero below
    mov r2, 0
    div r1, r2          ; 7 / 0 -> trap if the branch is broken
skip:
    cmp r0, r1
    je bad              ; 42 == 7 ? no: must fall through
    jmp end             ; forward jump over a nested label
    mov r2, 0
    div r1, r2          ; trap if jmp is broken
mid:
    mov r2, 0
    div r1, r2          ; trap if we land here
end:
    mov r3, 2
    shl r3, 4           ; 32
    add r3, 10          ; 42
    cmp r3, r0          ; equal
    jne bad
    ret
bad:
    mov r2, 0
    div r1, r2          ; trap if any branch went wrong
