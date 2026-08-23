#!/bin/bash
cd "$(dirname "$0")/.."
tim() { local s=$(date +%s%N); "$@" >/dev/null 2>&1; local e=$(date +%s%N); echo $(( (e-s)/1000000 )); }
echo "== MASM benchmark =="
python3 -c "print('.section .text\n#export _start\n_start:\n' + ' nop\n'*100000 + ' mov r1, 42\n mov r0, 60\n syscall\n')" > /tmp/bench_nop.s
python3 -c "
lines = ['.section .text', '#export _start', '_start:']
for k in range(30000):
    lines.append('mov r1, %d' % (k & 255))
    lines.append('add r1, %d' % (k & 15))
    lines.append('cmp r1, 250')
lines += ['mov r0, 60', 'syscall']
print('\n'.join(lines))" > /tmp/bench_real.s

ms0=$(tim ./masm -p X26A -a x86_64 /tmp/bench_nop.s -o /tmp/bn0.o)
ms1=$(tim ./masm -O -p X26A -a x86_64 /tmp/bench_nop.s -o /tmp/bn1.o)
s0=$(stat -c %s /tmp/bn0.o); s1=$(stat -c %s /tmp/bn1.o)
echo "100000 nop  : X26A ${ms0}ms ${s0}B  |  X26A -O ${ms1}ms ${s1}B"

ms2=$(tim ./masm -p X26A -a x86_64 /tmp/bench_real.s -o /tmp/br0.o)
ms3=$(tim ./masm -O -p X26A -a x86_64 /tmp/bench_real.s -o /tmp/br1.o)
echo "90000 instr : X26A ${ms2}ms  |  -O ${ms3}ms"

ms4=$(tim ./masm -a arm64 /tmp/bench_real.s -o /tmp/brr.o)
ms5=$(tim ./masm -a rv64gc /tmp/bench_real.s -o /tmp/brv.o)
ms6=$(tim ./masm -p X26A -a wasm32 /tmp/bench_nop.s -o /tmp/bw.wasm)
echo "90000 instr : arm64 ${ms4}ms  rv64gc ${ms5}ms"
echo "100000 nop  : wasm32 ${ms6}ms"
