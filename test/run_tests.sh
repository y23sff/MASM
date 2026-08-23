#!/bin/sh
# MASM regression suite: builds masm and checks every arch/format combination.
# needs: gcc, ld, python3, node (wasm tests only)
set -e
cd "$(dirname "$0")/.."
PASS=0
FAIL=0
ok()   { echo "  ok  - $1"; PASS=$((PASS+1)); }
bad()  { echo "  FAIL- $1"; FAIL=$((FAIL+1)); }

echo "== build =="
gcc main.c -O2 -Wall -Wextra -Wno-unused-parameter -o masm
ok "build (no warnings hidden)"

echo "== x86_64 / elf64 (link + run) =="
./masm -a x86_64 -f elf64 test/test.s -o /tmp/vex_t64.o 2>/dev/null
ld /tmp/vex_t64.o -o /tmp/vex_t64
[ "$(/tmp/vex_t64)" = "Hello, World!" ] && ok "hello world x86_64" || bad "hello world x86_64"

echo "== x86 / elf32 (link + run) =="
./masm -a x86 -f elf32 test/x86.s -o /tmp/vex_t32.o 2>/dev/null
ld -m elf_i386 /tmp/vex_t32.o -o /tmp/vex_t32
[ "$(/tmp/vex_t32)" = "Hello, 32bit!" ] && ok "hello world x86 (i386)" || bad "hello world x86 (i386)"

echo "== arm / elf32 (byte exact + relocs) =="
./masm -a arm test/arm.s -o /tmp/vex_arm.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_arm.o 40 1 \
  "04001fe50600000004101fe50700000090010ce00c00a0e12a0050e30100000a04001fe50000000004002de504109de4000000ea0000a0e10ef0a0e1" \
  && ok "arm a32 encodings" || bad "arm a32 encodings"
./masm -a arm test/arm_data.s -o /tmp/vex_armd.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_armd.o 40 1 - 4:2 12:2 28:2 \
  && ok "arm literal pool relocs (R_ARM_ABS32)" || bad "arm literal pool relocs"

echo "== arm table extras (shifts, setcc) =="
./masm -a arm test/arm_shift.s -o /tmp/vex_ash.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_ash.o 40 1 "1001a0e1a201a0e1e300a0e1e40fa0e1010050e10020a0e30120a0030ef0a0e1" \
  && ok "arm shifts + setcc" || bad "arm shifts + setcc"
./masm -a arm64 test/a64_more.s -o /tmp/vex_am.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_am.o 183 2 "804682d20020c09300f0c0932120c09a42fc43931f0001ebe3179f9a2414819ac0035fd6" \
  && ok "arm64 ror/rol(extr) + sbfm + cset + csinc" || bad "arm64 table extras"

echo "== arm64 / elf64 (byte exact + relocs) =="
./masm -a arm64 test/arm64.s -o /tmp/vex_a64.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_a64.o 183 2 \
  "c00080d2e10080d2007c019b1fa800f140000054000080d2e00f1ff8e14750f8020000141f2003d5c0035fd6" \
  && ok "arm64 a64 encodings" || bad "arm64 a64 encodings"
./masm -a arm64 test/arm64_data.s -o /tmp/vex_a64d.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_a64d.o 183 2 - 0:274:0 4:274:0 12:274:3 \
  && ok "arm64 adr relocs (R_AARCH64_ADR_PREL_LO21)" || bad "arm64 adr relocs"

echo "== riscv rv64gc / rv32gc (byte exact + relocs) =="
./masm -a rv64gc test/rv.s -o /tmp/vex_rv64.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_rv64.o 243 2 "13056000930570003305b5026304b50013050000130101ff2330a10083350100130101016f0080001300000067800000" \
  && ok "rv64gc encodings" || bad "rv64gc encodings"
./masm -a rv32gc test/rv.s -o /tmp/vex_rv32.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_rv32.o 243 1 "13056000930570003305b5026304b50013050000130101ff2320a10083250100130101016f0080001300000067800000" \
  && ok "rv32gc encodings (sw/lw push)" || bad "rv32gc encodings"
./masm -a rv64gc test/rv_data.s -o /tmp/vex_rvd.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_rvd.o 243 2 - 0:26:0 4:27:0 8:26:0 12:27:0 20:26:3 24:27:3 \
  && ok "riscv HI20/LO12_I label addressing" || bad "riscv relocs"

echo "== coff (structure, all archs) =="
./masm -a x86_64 -f coff test/test.s     -o /tmp/vex_c64.obj  2>/dev/null
./masm -a x86    -f coff test/x86.s      -o /tmp/vex_c32.obj  2>/dev/null
./masm -a arm    -f coff test/arm_data.s -o /tmp/vex_carm.obj 2>/dev/null
./masm -a arm64  -f coff test/arm64_data.s -o /tmp/vex_ca64.obj 2>/dev/null
python3 test/check_obj.py coff /tmp/vex_c64.obj  0x8664 1 && ok "coff amd64"  || bad "coff amd64"
python3 test/check_obj.py coff /tmp/vex_c32.obj  0x14c  1 && ok "coff i386"   || bad "coff i386"
python3 test/check_obj.py coff /tmp/vex_carm.obj 0x1c0  3 && ok "coff arm"    || bad "coff arm"
python3 test/check_obj.py coff /tmp/vex_ca64.obj 0xaa64 3 && ok "coff arm64"  || bad "coff arm64"

echo "== wasm32 / wasm64 (validate + execute) =="
if command -v node >/dev/null; then
  ./masm -a wasm32 test/wasm.s -o /tmp/vex_w32.wasm 2>/dev/null
  ./masm -a wasm64 test/wasm.s -o /tmp/vex_w64.wasm 2>/dev/null
  ./masm -a wasm32 test/wasm_loop.s -o /tmp/vex_wl32.wasm 2>/dev/null
  ./masm -a wasm64 test/wasm_loop.s -o /tmp/vex_wl64.wasm 2>/dev/null
  node -e '
const fs=require("fs");
for (const f of ["/tmp/vex_w32.wasm","/tmp/vex_w64.wasm","/tmp/vex_wl32.wasm","/tmp/vex_wl64.wasm"]) {
  const m=new WebAssembly.Module(fs.readFileSync(f));
  new WebAssembly.Instance(m,{}).exports.main();
}
console.log("  ok  - wasm modules validate, branches + loops hold");
' && PASS=$((PASS+1)) || bad "wasm"
else
  echo "  skip- node not found"
fi

echo "== X26A parser (include / ifdef / extern / export) =="
./masm -p X26A -a x86_64 test/x26a.s -o /tmp/vex_xa1.o 2>/dev/null
./masm -p X26A -a x86_64 test/x26a_ext.s -o /tmp/vex_xa2.o 2>/dev/null
ld /tmp/vex_xa1.o /tmp/vex_xa2.o -o /tmp/vex_xa
rc=0; /tmp/vex_xa || rc=$?
[ "$rc" -eq 70 ] && ok "x26a nested include+dedupe+ifdef+extern link" || bad "x26a link"
readelf -s /tmp/vex_xa1.o 2>/dev/null | grep -q "UND plus1" \
  && ok "x26a extern symbol is GLOBAL UND" || bad "x26a extern symbol"
./masm -p X26A -a x86_64 -f coff test/x26a.s -o /tmp/vex_xa1.obj 2>/dev/null
python3 test/check_obj.py coff /tmp/vex_xa1.obj 0x8664 1 13 \
  && ok "x26a coff symbols" || bad "x26a coff symbols"
printf '#frobnicate\n' > /tmp/vex_dir.s
./masm -p X26A /tmp/vex_dir.s -o /tmp/vex_x.o 2>/dev/null && bad "unknown directive not caught" \
  || ok "unknown directive caught"
printf '.globl _start\n' > /tmp/vex_gl.s
./masm -p X26A /tmp/vex_gl.s -o /tmp/vex_x.o 2>/dev/null && bad ".globl not rejected" \
  || ok ".globl rejected in x26a"
printf '#export nothere\n' > /tmp/vex_nx.s
./masm -p X26A /tmp/vex_nx.s -o /tmp/vex_x.o 2>/dev/null && bad "bad export not caught" \
  || ok "export of undefined label caught"

echo "== auto width (keyword yok, arch kendi secer) =="
./masm -a x86_64 test/autowidth.s -o /tmp/vex_aw64.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_aw64.o 62 2 "48b80000000001000000c3" \
  && ok "x86_64 auto qword (movabs)" || bad "x86_64 auto width"
./masm -a x86 test/autowidth.s -o /tmp/vex_aw32.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_aw32.o 3 1 "b800000000c3" \
  && ok "x86 auto dword" || bad "x86 auto width"

echo "== X26A builtin ARCH_/FORMAT_ defines =="
./masm -p X26A -a x86_64 test/x26a_arch.s -o /tmp/vex_ab64.o 2>/dev/null
ld /tmp/vex_ab64.o -o /tmp/vex_ab64
rc=0; /tmp/vex_ab64 || rc=$?
[ "$rc" -eq 42 ] && ok "ARCH_X86_64 branch taken (exit 42)" || bad "ARCH_X86_64"
./masm -p X26A -a arm64 test/x26a_arch.s -o /tmp/vex_abarm.o 2>/dev/null
python3 test/check_obj.py elf /tmp/vex_abarm.o 183 2 "600580d2c0035fd6" \
  && ok "ARCH_ARM64 branch, ARCH_X86_64 ignored" || bad "ARCH_ARM64"
./masm -p X26A -a x86_64 -f coff test/x26a_arch.s -o /tmp/vex_abac.obj 2>/dev/null
python3 -c "
import struct, sys
b = open('/tmp/vex_abac.obj','rb').read()
rawsz = struct.unpack_from('<I', b, 36)[0]
rawptr = struct.unpack_from('<I', b, 40)[0]
t = b[rawptr:rawptr+rawsz]
want = bytes.fromhex('48bf2a0000000000000048b83c000000000000000f0590')
sys.exit(0 if t == want else 1)
" && ok "FORMAT_COFF branch adds nop, FORMAT_ELF64 does not" || bad "FORMAT_COFF"

echo "== X26A #else =="
./masm -p X26A -a x86_64 test/x26a_else.s -o /tmp/vex_el.o 2>/dev/null
ld /tmp/vex_el.o -o /tmp/vex_el
rc=0; /tmp/vex_el || rc=$?
if objdump -d /tmp/vex_el | grep -q int3; then bad "x26a else took wrong branch"
else [ "$rc" -eq 7 ] && ok "x26a else picks the right branch" || bad "x26a else"
fi
printf '#ifdef NOPE\n#frobnicate\n#endif\n' > /tmp/vex_strict.s
./masm -p X26A /tmp/vex_strict.s -o /tmp/vex_x.o 2>/dev/null && bad "unknown directive in dead branch not caught" \
  || ok "unknown directive caught even in dead branch"

echo "== include = pure paste + auto guard, single object, no linking =="
./masm -p X26A -a x86_64 test/x26a_solo.s -o /tmp/masm_solo.o 2>/dev/null
ld /tmp/masm_solo.o -o /tmp/masm_solo
rc=0; /tmp/masm_solo || rc=$?
[ "$rc" -eq 36 ] && ok "x26a include pastes once, one object, zero linking" || bad "x26a solo include"
./masm -a x86_64 test/inc_classic.s -o /tmp/masm_ic.o 2>/dev/null
ld /tmp/masm_ic.o -o /tmp/masm_ic
rc=0; /tmp/masm_ic || rc=$?
[ "$rc" -eq 15 ] && ok "classic x26 include, file relative, entry after pasted code" || bad "classic include"
( cd test && ../masm -a x86_64 inc_classic.s -o /tmp/masm_ic2.o ) 2>/dev/null \
  && ok "include works from any working directory" || bad "include cwd"

echo "== libc (x86_64, gercek printf cagrisi) =="
if command -v gcc >/dev/null; then
  ./masm -p X26A -a x86_64 test/libc_printf.s -o /tmp/masm_libc.o 2>/dev/null
  gcc -no-pie /tmp/masm_libc.o -o /tmp/masm_libc 2>/dev/null
  rc=0; out=$(/tmp/masm_libc) || rc=$?
  [ "$rc" -eq 42 ] && [ "$out" = "masm libc test: 42" ] \
    && ok "libc printf interop + exit 42" || bad "libc printf"
else
  echo "  skip- gcc yok"
fi

echo "== optimizer -O =="
./masm -p X26A -a x86_64 test/deadcode.s -o /tmp/masm_dc0.o 2>/dev/null
./masm -O -p X26A -a x86_64 test/deadcode.s -o /tmp/masm_dc1.o 2>/dev/null
ld /tmp/masm_dc1.o -o /tmp/masm_dc1
rc=0; /tmp/masm_dc1 || rc=$?
sz0=$(readelf -SW /tmp/masm_dc0.o | awk '$3==".text"{print "0x"$7}' | python3 -c "import sys;print(int(sys.stdin.read().strip(),16))")
sz1=$(readelf -SW /tmp/masm_dc1.o | awk '$3==".text"{print "0x"$7}' | python3 -c "import sys;print(int(sys.stdin.read().strip(),16))")
if [ "$rc" -eq 7 ] && [ "$sz1" -lt "$sz0" ]; then ok "deadcode silindi (text $sz0 -> $sz1) + davranis ayni (exit 7)"
else bad "deadcode optimizer"; fi

python3 - <<'PY'
lines = ['.section .text', '#export _start', '_start:', ' mov r1, 5']
lines += [' nop'] * 100
lines += [' add r1, 1', ' mov r0, 60', ' syscall']
open('/tmp/masm_nop.s', 'w').write('\n'.join(lines) + '\n')
PY
./masm -O -p X26A -a x86_64 /tmp/masm_nop.s -o /tmp/masm_np1.o 2>/dev/null
ld /tmp/masm_np1.o -o /tmp/masm_np1
rc=0; /tmp/masm_np1 || rc=$?
sz=$(readelf -SW /tmp/masm_np1.o | awk '$3==".text"{print "0x"$7}' | python3 -c "import sys;print(int(sys.stdin.read().strip(),16))")
if [ "$rc" -eq 6 ] && [ "$sz" -lt 90 ]; then ok "100 nop -> dongu (text ${sz}B, 112 olurdu) + exit 6"
else bad "nop loop optimizer ($sz, $rc)"; fi

for a in arm arm64 rv64gc; do
  ./masm -O -p X26A -a $a /tmp/masm_nop.s -o /tmp/masm_np_$a.o 2>/dev/null
  sz=$(readelf -SW /tmp/masm_np_$a.o | awk '$3==".text"{print "0x"$7}' | python3 -c "import sys;print(int(sys.stdin.read().strip(),16))")
  [ "$sz" -lt 100 ] && ok "nop dongu $a: text ${sz}B (400 olurdu)" || bad "nop dongu $a"
done

echo "== ozel kayitlar tmp/sp/fp =="
./masm -p X26A -a x86_64 test/regs.s -o /tmp/masm_rg.o 2>/dev/null
ld /tmp/masm_rg.o -o /tmp/masm_rg
rc=0; /tmp/masm_rg || rc=$?
[ "$rc" -eq 42 ] && ok "tmp x86_64: mov tmp + aktarim, exit 42" || bad "tmp x86_64"
for a in x86 arm arm64 rv64gc; do
  ./masm -p X26A -a $a test/regs.s -o /tmp/masm_rg_$a.o 2>/dev/null || bad "regs $a"
done && ok "tmp/sp/fp tum tablolar derleniyor (x86 arm arm64 rv64gc)"
printf '.section .text\n#export _start\n_start:\n mov tmp, 9\n mov fp, tmp\n mov r2, fp\n mov sp, tmp\n ret\n' > /tmp/masm_fpreg.s
./masm -a arm64 /tmp/masm_fpreg.s -o /tmp/masm_fp.o 2>/dev/null
python3 test/check_obj.py elf /tmp/masm_fp.o 183 2 "310180d2fd0311aae2031daaff0311aac0035fd6" 2>/dev/null \
  && ok "arm64 fp=x29 sp=x31 tmp=x17 bayt-dogru" || bad "arm64 fp/sp baytlari"
printf '.section .text\n_start:\n mov tmp, 5\n mov r1, tmp\n ret\n' > /tmp/masm_wtmp.s
./masm -a wasm32 /tmp/masm_wtmp.s -o /tmp/masm_wt.wasm 2>/dev/null
node -e 'new WebAssembly.Instance(new WebAssembly.Module(require("fs").readFileSync("/tmp/masm_wt.wasm")),{}).exports.main()' 2>/dev/null \
  && ok "wasm tmp = local8" || bad "wasm tmp"
printf '.section .text\n_start:\n mov sp, 5\n ret\n' > /tmp/masm_wsp.s
./masm -a wasm32 /tmp/masm_wsp.s -o /tmp/masm_ws.wasm 2>/dev/null && bad "wasm sp yakalanmadi" \
  || ok "wasm sp/fp reddediliyor"

echo "== error paths =="
echo 'jmp nowhere' | sed 's/^/.section .text\n_start:\n/' > /tmp/vex_undef.s
./masm -a x86_64 /tmp/vex_undef.s -o /tmp/vex_x.o 2>/dev/null && bad "undefined label not caught" \
  || ok "undefined label caught"
printf '.section .text\n_start:\n nop\nL1:\n nop\n b L1\nM:\n nop\n b L1\n' > /tmp/vex_back.s
./masm -a wasm32 /tmp/vex_back.s -o /tmp/vex_x.wasm 2>/dev/null && bad "wasm closed loop not caught" \
  || ok "wasm second back edge after close caught"

echo
echo "passed: $PASS  failed: $FAIL"
[ "$FAIL" -eq 0 ]
