#include "x26a_inc2.s"
#ifndef X26A_INC
#define X26A_INC
#export triple
triple:
    mov r0, r1
    shl r0, 1
    add r0, r1
    ret
#endif
