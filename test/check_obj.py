#!/usr/bin/env python3
"""check_obj.py: verify vex ELF/COFF object structure and .text bytes."""
import struct, sys

def rd(p): return open(p, 'rb').read()

def check_elf(path, machine, elfclass, text_hex, relocs):
    b = rd(path)
    assert b[:4] == b'\x7fELF', 'magic'
    assert b[4] == elfclass, f'class {b[4]} != {elfclass}'
    m = struct.unpack_from('<H', b, 18)[0]
    assert m == machine, f'machine {m} != {machine}'
    if elfclass == 2:
        shoff = struct.unpack_from('<Q', b, 40)[0]
        shentsz, shnum, shstrndx = struct.unpack_from('<HHH', b, 58)
        sh_fmt = '<IIQQQQIIQQ'
        names_at = lambda sh: sh[0]
    else:
        shoff = struct.unpack_from('<I', b, 32)[0]
        shentsz, shnum, shstrndx = struct.unpack_from('<HHH', b, 46)
        sh_fmt = '<IIIIIIIIII'
        names_at = lambda sh: sh[0]
    shs = []
    for i in range(shnum):
        sh = struct.unpack_from(sh_fmt, b, shoff + i * shentsz)
        shs.append(sh)
    shstr = shs[shstrndx]
    strtab = b[shstr[4]:shstr[4] + shstr[5]]
    def nm(sh):
        o = names_at(sh)
        e = strtab.index(b'\0', o)
        return strtab[o:e].decode()
    text = next(s for s in shs if nm(s) == '.text')
    got = b[text[4]:text[4] + text[5]]
    if text_hex == '-':
        want = got
    else:
        want = bytes.fromhex(text_hex.replace(' ', '').replace('\n', ''))
    if got != want:
        print('text mismatch:')
        print('  want', want.hex())
        print('  got ', got.hex())
        sys.exit(1)
    # relocations
    relsh = next((s for s in shs if nm(s).startswith('.rel')), None)
    got_relocs = []
    if relsh:
        d = b[relsh[4]:relsh[4] + relsh[5]]
        ent = 24 if elfclass == 2 else 8
        for i in range(0, len(d), ent):
            if elfclass == 2:
                off, info, add = struct.unpack_from('<QQq', d, i)
                got_relocs.append((off, info & 0xFFFFFFFF, add))
            else:
                off, info = struct.unpack_from('<II', d, i)
                got_relocs.append((off, info & 0xFF, None))
    exp = []
    for r in relocs:
        v = [int(x) for x in r]
        exp.append((v[0], v[1], v[2]) if len(v) > 2 else (v[0], v[1], None))
    if got_relocs != exp:
        print(f'reloc mismatch: got {got_relocs} want {exp}')
        sys.exit(1)
    print(f'OK {path}: text {len(want)}B, {len(exp)} relocs verified')

def check_coff(path, machine, nrels, symcount=10):
    b = rd(path)
    m, nsec, _, symptr, nsyms = struct.unpack_from('<HHIII', b, 0)
    assert m == machine, f'machine {m:04x} != {machine:04x}'
    assert nsec == 4, f'nsec {nsec}'
    assert nsyms == symcount, f'nsyms {nsyms} != {symcount}'
    txt = struct.unpack_from('<40s', b, 20)[0]
    name = txt[:8]
    vsz, va, rawsz = struct.unpack_from('<III', txt, 8)
    rawptr, relptr = struct.unpack_from('<II', txt, 20)
    nrel = struct.unpack_from('<H', txt, 32)[0]
    assert name == b'.text\0\0\0', name
    assert nrel == nrels, f'nrel {nrel} != {nrels}'
    # walk relocations: sym index must be a section symbol (1,3,5,7)
    for i in range(nrel):
        va_, sym, typ = struct.unpack_from('<IIH', b, relptr + i * 10)
        assert sym in (1, 3, 5, 7) or sym >= 9, f'reloc {i}: sym {sym}'
        assert typ != 0, f'reloc {i}: type 0'
    assert struct.unpack_from('<I', b, symptr + nsyms * 18)[0] >= 4, 'strtab size'
    print(f'OK {path}: machine {machine:04x}, {nrels} relocs, {nsyms} symbols')

if __name__ == '__main__':
    kind = sys.argv[1]
    if kind == 'elf':
        # elf FILE MACHINE CLASS TEXT_HEX RELOC...  (RELOC=off:type[:addend])
        check_elf(sys.argv[2], int(sys.argv[3]), int(sys.argv[4]), sys.argv[5],
                  [r.split(':') for r in sys.argv[6:]])
    else:
        check_coff(sys.argv[2], int(sys.argv[3], 0), int(sys.argv[4]),
                   int(sys.argv[5]) if len(sys.argv) > 5 else 10)
