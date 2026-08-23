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

parsers:

* X26
* X26A

# LLM Policy
masm accepts llm pr requests not pushes
use ai i dont care use it as a tool i do! 
for tables and some hardcoded thinks

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

### X26 syntax supports:

```s
.globl <label>
<label>:
.section <section>
<label>: resb
<label>: string
<label>: constString
<label>: number
<label>: constNumber
```

### X26 static bit width selection

X26 uses static bit width selection
```s
mov qword r0 50
mov dword r0 40
mov word r1 50
mov byte r1 40

mov r0 50 ; no bad boy
```

### here is the first version of the X26A syntax:
```c
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
```
### X26A syntax has speacial defs

the speacial defs for the X26A is to make easy to easy way to learn target arch or format
```s
_FORMAT_<FORMAT NAME>
_ARCH_<ARCH NAME>
```

### X26A bit width selection

X26A has a automatic bit width selection for example
```s
mov qword r0 r1 ; can be used
mov r0 r1 ; recommended it can be casted in to 32 16 8 and more bits
```
this is a good feature if you are going to write code for multiaple arch with diffirent bit widths

### X26A optimizer !!!NEW(v0.1.0 stable) for testing2

X26A now has a optimizer with
* dead code cleaning
* adding loops

#### THE PROBLEM WITH X26A OPTIMIZER

while the dead code can be used to jumped from another program
the main problem is the 1000 nops optimizer make the machine behave diffirently

## FORMAT MODULES & ARCH MODULES

these modules should be table driven or not 
for arch modules you should add masm instruction encoder look from x86_64.h and register it
for format modules you should add link function and register it
about rel if you dont like any of it just write it in to enum in core.h

# BUG FIXES
fixed (v0.1.0 stable)
1. we fixed 4 gib bss bug
2. parser errors added
3. dynamic relocs and sym added removed arrays

# PROJECTS
we are working on GALC(grammar languange creator) which uses this project as a backend

# COMMIT HELL
my original accounts name changed it caused a bug i think so yusufesadaykan-spec is me

# DEVELOPMENT
this is the public version of the MASM
we are building the MASM in a private repo

# LICENSE
MASM currently using *MIT* license

