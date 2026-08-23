# MASM v0.1.0 stable
masm is a modular cross arch and platform assembler for embeded devs end os devs
we belive that every sofware should be modular so its going to be easy to add new features
we belive that every format or arch should be supported with assemblers so the new format
or arch will be grow quikly as possible we dont want any bloat in our code no dependcies

# Supports
masm supports
* x86-x86_64
* wasm
* arm-arm64
* rv64gc-rv32gc
* more will be come
formats:
* elf64-elf32
* coff
* wasm32 wasm64
parser:
* X26
* X26A

# LLM Policy
masm accepts llm pr requests not pushes
use ai i dont care

# MODULES

masm is builded with modules which is in these folders
    /syntax
    /arch
    /format
but you need to include that in to main file and use register to use

## SYNTAX MODULES

masm supports new assembly syntaxes so you can build your own
we are currently using X26 as default but we are currently testing X26A syntax
which is more modern and has big features

X26 syntax supports:

    .globl <label>
    <label>:
    .section <section>
    <label>: resb
    <label>: string
    <label>: constString
    <label>: number
    <label>: constNumber

X26 static bit width selection

    x26 syntax uses bit width for example

    mov qword r0 50
    mov dword r0 40
    mov word r1 50
    mov byte r1 40

here is the first version of the X26A syntax:

    #ifndef <def>
    #define <def>
    #ifdef <def>
    #endif
    #extern <label>
    #export <label>
    <label>:
    .section <section>
    <label>: resb
    <label>: string
    <label>: constString
    <label>: number
    <label>: constNumber

X26A syntax has speacial defs
    _FORMAT_<FORMAT NAME>
    _ARCH_<ARCH NAME>

X26A bit width selection

    x26a have auto bit width selection for cross arch programs for example

mov r0 50 | x86    | mov eax, 50

mov r0 50 | x86_64 | mov rax, 50

these are same for the arm arm64 etc...

X26A optimizer !!!NEW(v0.1.0 stable)

    think a assembly file with 1000 nops when we assemble it with X26A Optimzer

    we should get 
    push tmp
    mov tmp, N
    __masm_nop<k>:
    nop
    dec tmp
    test tmp, tmp
    jnz __masm_nop<k>
    pop tmp

    and X26A optimzer will be getting more features

## FORMAT MODULES & ARCH MODULES

these modules should be table driven or not 
for arch modules you should add masm instruction encoder look from x86_64.h and register it
for format modules you should add link function and register it
about rel if you dont like any of it just write it in to enum in core.h

# BUG FIXES
fixed
1. we fixed 4 gib bss bug
2. parser errors added
3. dynamic relocs and sym added removed tables
commiting now! so wait 1 hour or so

# PROJECTS
we are working on GALC(grammar languange creator) which uses this project as a backend

# COMMIT HELL
my original accounts name changed it caused a bug i think so yusufesadaykan-spec is me not others
btw we are commiting on a other repo so if there is less commits do not care about that
