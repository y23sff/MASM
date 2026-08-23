#ifndef MASM_ELF_H
#define MASM_ELF_H

#include "../core.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ET_REL     1
#define EM_386     3
#define EM_ARM     40
#define EM_X86_64  62
#define EM_AARCH64 183
#define EM_RISCV   243

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_NOBITS   8
#define SHT_REL      9

#define SHF_WRITE    0x1
#define SHF_ALLOC    0x2
#define SHF_EXECINSTR 0x4
#define SHF_INFO_LINK 0x40

#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STT_NOTYPE 0
#define STT_FUNC   2
#define STT_SECTION 3
#define STT_FILE   4
#define SHN_UNDEF  0
#define SHN_ABS    0xFFF1
#define EF_ARM_EABI_VER5 0x05000000

#define R_386_32            1
#define R_386_PC32          2
#define R_ARM_ABS32         2
#define R_ARM_CALL          28
#define R_ARM_JUMP24        29
#define R_X86_64_PC32       2
#define R_AARCH64_ABS32     258
#define R_AARCH64_CONDBR19  264
#define R_AARCH64_ADR_PREL_LO21 274
#define R_AARCH64_JUMP26    282
#define R_AARCH64_CALL26    283
#define R_RISCV_BRANCH      16
#define R_RISCV_JAL         17
#define R_RISCV_HI20        26
#define R_RISCV_LO12_I      27

#define IDX_TEXT     1
#define IDX_RODATA   2
#define IDX_DATA     3
#define IDX_BSS      4
#define IDX_SYMTAB   5
#define IDX_STRTAB   6
#define IDX_SHSTRTAB 7
#define IDX_REL      8

static uint8_t elf_str[8192];
static uint32_t elf_str_used;
static uint8_t elf_shstr[256];
static uint32_t elf_shstr_used;
static uint8_t *elf_sym;
static uint32_t elf_sym_used;
static uint32_t elf_nsym;
static uint8_t *elf_rel;
static uint32_t elf_rel_used;

static uint8_t elf_text[1 << 18];
static uint8_t elf_rodata[1 << 18];
static uint8_t elf_data[1 << 16];

static int elf_is64(const char *a) {
  return strcmp(a, "x86_64") == 0 || strcmp(a, "arm64") == 0 ||
         strcmp(a, "rv64gc") == 0;
}
static int elf_supported(const char *a) {
  return elf_is64(a) || strcmp(a, "x86") == 0 || strcmp(a, "arm") == 0 ||
         strcmp(a, "rv32gc") == 0;
}
static int elf_rela32(const char *a) { return strcmp(a, "rv32gc") == 0; }
static int elf_machine(const char *a) {
  if (strcmp(a, "x86_64") == 0)
    return EM_X86_64;
  if (strcmp(a, "x86") == 0)
    return EM_386;
  if (strcmp(a, "arm") == 0)
    return EM_ARM;
  if (strcmp(a, "rv32gc") == 0 || strcmp(a, "rv64gc") == 0)
    return EM_RISCV;
  return EM_AARCH64;
}
static uint32_t elf_flags(const char *a) {
  return strcmp(a, "arm") == 0 ? EF_ARM_EABI_VER5 : 0;
}

static int elf_reltype(const char *a, uint8_t t) {
  if (strcmp(a, "x86_64") == 0)
    return t == REL_PC32 ? R_X86_64_PC32 : -1;
  if (strcmp(a, "x86") == 0)
    return t == REL_ABS32 ? R_386_32 : t == REL_PC32 ? R_386_PC32 : -1;
  if (strcmp(a, "arm") == 0)
    return t == REL_ABS32 ? R_ARM_ABS32
           : t == REL_BRJMP ? R_ARM_JUMP24
           : t == REL_BRCALL ? R_ARM_CALL : -1;
  if (strcmp(a, "rv32gc") == 0 || strcmp(a, "rv64gc") == 0)
    return t == RV_HI20 ? R_RISCV_HI20
           : t == RV_LO12 ? R_RISCV_LO12_I
           : t == RV_JAL ? R_RISCV_JAL
           : t == RV_BRANCH ? R_RISCV_BRANCH : -1;
  return t == REL_ABS32 ? R_AARCH64_ABS32
         : t == REL_ADR21 ? R_AARCH64_ADR_PREL_LO21
         : t == REL_BCC19 ? R_AARCH64_CONDBR19
         : t == REL_BRJMP ? R_AARCH64_JUMP26
         : t == REL_BRCALL ? R_AARCH64_CALL26 : -1;
}

static void elf_w8(FILE *o, uint32_t v) { fputc((int)(v & 0xFF), o); }
static void elf_w16(FILE *o, uint32_t v) {
  elf_w8(o, v);
  elf_w8(o, v >> 8);
}
static void elf_w32(FILE *o, uint32_t v) {
  elf_w16(o, v);
  elf_w16(o, v >> 16);
}
static void elf_w64(FILE *o, uint64_t v) {
  elf_w32(o, (uint32_t)v);
  elf_w32(o, (uint32_t)(v >> 32));
}

static void elf_wr(FILE *o, const void *p, size_t n) {
  if (n && fwrite(p, 1, n, o) != n) {
    fprintf(stderr, "masm: elf: write failed\n");
    exit(1);
  }
}

static void elf_pad(FILE *o, long to) {
  long p = ftell(o);
  while (p < to) {
    fputc(0, o);
    p++;
  }
}

static uint32_t elf_addstr(uint8_t *buf, uint32_t *used, uint32_t cap,
                           const char *s) {
  uint32_t off = *used;
  size_t n = strlen(s) + 1;
  if (off + n > cap) {
    fprintf(stderr, "masm: elf: string table full\n");
    exit(1);
  }
  memcpy(buf + off, s, n);
  *used += (uint32_t)n;
  return off;
}

static const char *elf_base(const char *p) {
  const char *s = p ? p : "source.s";
  for (const char *q = s; *q; q++)
    if (*q == '/')
      s = q + 1;
  return s;
}

static long elf_slurp(FILE *f, uint8_t *dst, long cap) {
  if (!f)
    return 0;
  fflush(f);
  long sz = ftell(f);
  if (sz < 0)
    sz = 0;
  if (sz > cap) {
    fprintf(stderr, "masm: elf: section too large\n");
    exit(1);
  }
  rewind(f);
  if (sz && fread(dst, 1, (size_t)sz, f) != (size_t)sz) {
    fprintf(stderr, "masm: elf: read failed\n");
    exit(1);
  }
  return sz;
}

static uint32_t elf_r32(const uint8_t *b, uint64_t at) {
  return (uint32_t)b[at] | ((uint32_t)b[at + 1] << 8) |
         ((uint32_t)b[at + 2] << 16) | ((uint32_t)b[at + 3] << 24);
}
static void elf_w32b(uint8_t *b, uint64_t at, uint32_t v) {
  b[at] = (uint8_t)v;
  b[at + 1] = (uint8_t)(v >> 8);
  b[at + 2] = (uint8_t)(v >> 16);
  b[at + 3] = (uint8_t)(v >> 24);
}

static void elf_patch(const char *arch, uint8_t *buf, rel *r) {
  int64_t a = r->addend;
  if (strcmp(arch, "arm") == 0 &&
      (r->type == REL_BRJMP || r->type == REL_BRCALL)) {

    uint32_t w = elf_r32(buf, r->adrs);
    w = (w & 0xFF000000u) | ((uint32_t)(a >> 2) & 0x00FFFFFFu);
    elf_w32b(buf, r->adrs, w);
    return;
  }
  elf_w32b(buf, r->adrs, (uint32_t)(int32_t)a);
}

static uint32_t elf_secsym(SECTION s) {
  switch (s) {
  case SECTION_TEXT:
    return 2;
  case SECTION_RODATA:
    return 3;
  case SECTION_DATA:
    return 4;
  case SECTION_BSS:
    return 5;
  default:
    return 0;
  }
}

static uint32_t elf_sym_index(context *ctx, rel *r) {
  if (r->section != SECTION_NULL || !r->name)
    return elf_secsym(r->section);
  for (int i = 0; i < ctx->nsyms; i++)
    if (ctx->syms[i].kind == SYM_EXTERN &&
        strcmp(ctx->syms[i].name, r->name) == 0)
      return (ctx->nsyms == 0 ? 7u : 6u) + (uint32_t)i;
  fprintf(stderr, "masm: elf: extern symbol '%s' not declared\n", r->name);
  exit(1);
}

static void elf_addsym64(uint32_t name, uint8_t info, uint16_t shndx,
                         uint64_t value) {
  uint8_t *b = elf_sym + elf_sym_used;
  memset(b, 0, 24);
  b[0] = (uint8_t)name;
  b[1] = (uint8_t)(name >> 8);
  b[2] = (uint8_t)(name >> 16);
  b[3] = (uint8_t)(name >> 24);
  b[4] = info;
  b[6] = (uint8_t)shndx;
  b[7] = (uint8_t)(shndx >> 8);
  for (int i = 0; i < 8; i++)
    b[8 + i] = (uint8_t)(value >> (8 * i));
  elf_sym_used += 24;
  elf_nsym++;
}

static void elf_addsym32(uint32_t name, uint8_t info, uint16_t shndx,
                         uint32_t value) {

  uint8_t *b = elf_sym + elf_sym_used;
  memset(b, 0, 16);
  b[0] = (uint8_t)name;
  b[1] = (uint8_t)(name >> 8);
  b[2] = (uint8_t)(name >> 16);
  b[3] = (uint8_t)(name >> 24);
  for (int i = 0; i < 4; i++)
    b[4 + i] = (uint8_t)(value >> (8 * i));
  b[12] = info;
  b[14] = (uint8_t)shndx;
  b[15] = (uint8_t)(shndx >> 8);
  elf_sym_used += 16;
  elf_nsym++;
}

static void elf_link(context *ctx, char *output_file) {
  const char *arch = ctx->arch ? ctx->arch : "x86_64";
  if (!elf_supported(arch)) {
    fprintf(stderr, "masm: elf: arch '%s' not supported\n", arch);
    exit(1);
  }
  int c64 = elf_is64(arch);
  int r32rela = elf_rela32(arch);
  int rela = c64 || r32rela;
  long text_sz = elf_slurp(ctx->text_stream, elf_text, sizeof elf_text);
  long ro_sz = elf_slurp(ctx->rodata_stream, elf_rodata, sizeof elf_rodata);
  long da_sz = elf_slurp(ctx->data_stream, elf_data, sizeof elf_data);
  uint64_t bss_sz = ctx->bss_size ? *ctx->bss_size : 0;
  if (!c64 && bss_sz > 0xFFFFFFFFull) {
    fprintf(stderr, "masm: elf: bss too large for a 32 bit object\n");
    exit(1);
  }
  elf_str_used = 1;
  elf_shstr_used = 1;
  elf_sym_used = 0;
  elf_nsym = 0;
  elf_rel_used = 0;
  elf_str[0] = 0;
  elf_shstr[0] = 0;
  elf_sym = (uint8_t *)malloc((size_t)(ctx->nsyms + 16) * 24);
  elf_rel = (uint8_t *)malloc((size_t)(ctx->nrelocs + 1) * 24);
  if (!elf_sym || !elf_rel) {
    fprintf(stderr, "masm: elf: out of memory\n");
    exit(1);
  }
  elf_addstr(elf_shstr, &elf_shstr_used, sizeof elf_shstr, "");
  uint32_t n_text = elf_addstr(elf_shstr, &elf_shstr_used, sizeof elf_shstr, ".text");
  uint32_t n_ro = elf_addstr(elf_shstr, &elf_shstr_used, sizeof elf_shstr, ".rodata");
  uint32_t n_da = elf_addstr(elf_shstr, &elf_shstr_used, sizeof elf_shstr, ".data");
  uint32_t n_bss = elf_addstr(elf_shstr, &elf_shstr_used, sizeof elf_shstr, ".bss");
  uint32_t n_sym = elf_addstr(elf_shstr, &elf_shstr_used, sizeof elf_shstr, ".symtab");
  uint32_t n_str = elf_addstr(elf_shstr, &elf_shstr_used, sizeof elf_shstr, ".strtab");
  uint32_t n_shs = elf_addstr(elf_shstr, &elf_shstr_used, sizeof elf_shstr, ".shstrtab");
  uint32_t n_rel = elf_addstr(elf_shstr, &elf_shstr_used, sizeof elf_shstr,
                              rela ? ".rela.text" : ".rel.text");
  uint32_t s_file =
      elf_addstr(elf_str, &elf_str_used, sizeof elf_str, elf_base(ctx->input_name));
  uint32_t s_start =
      ctx->nsyms ? 0
                 : elf_addstr(elf_str, &elf_str_used, sizeof elf_str, "_start");
  if (c64) {
    elf_addsym64(0, 0, 0, 0);
    elf_addsym64(s_file, (STB_LOCAL << 4) | STT_FILE, SHN_ABS, 0);
    elf_addsym64(0, (STB_LOCAL << 4) | STT_SECTION, IDX_TEXT, 0);
    elf_addsym64(0, (STB_LOCAL << 4) | STT_SECTION, IDX_RODATA, 0);
    elf_addsym64(0, (STB_LOCAL << 4) | STT_SECTION, IDX_DATA, 0);
    elf_addsym64(0, (STB_LOCAL << 4) | STT_SECTION, IDX_BSS, 0);
  } else {
    elf_addsym32(0, 0, 0, 0);
    elf_addsym32(s_file, (STB_LOCAL << 4) | STT_FILE, SHN_ABS, 0);
    elf_addsym32(0, (STB_LOCAL << 4) | STT_SECTION, IDX_TEXT, 0);
    elf_addsym32(0, (STB_LOCAL << 4) | STT_SECTION, IDX_RODATA, 0);
    elf_addsym32(0, (STB_LOCAL << 4) | STT_SECTION, IDX_DATA, 0);
    elf_addsym32(0, (STB_LOCAL << 4) | STT_SECTION, IDX_BSS, 0);
  }
  uint32_t first_global = elf_nsym;
  if (ctx->nsyms == 0) {
    uint64_t sval = ctx->has_start ? ctx->start_off : 0;
    if (c64)
      elf_addsym64(s_start, (STB_GLOBAL << 4) | STT_FUNC, IDX_TEXT, sval);
    else
      elf_addsym32(s_start, (STB_GLOBAL << 4) | STT_FUNC, IDX_TEXT,
                   (uint32_t)sval);
  }
  for (int i = 0; i < ctx->nsyms; i++) {
    sym *us = &ctx->syms[i];
    uint32_t sn = elf_addstr(elf_str, &elf_str_used, sizeof elf_str, us->name);
    if (us->kind == SYM_EXTERN) {
      if (c64)
        elf_addsym64(sn, (STB_GLOBAL << 4) | STT_NOTYPE, SHN_UNDEF, 0);
      else
        elf_addsym32(sn, (STB_GLOBAL << 4) | STT_NOTYPE, SHN_UNDEF, 0);
    } else {
      uint16_t shn = us->section == SECTION_TEXT ? IDX_TEXT
                     : us->section == SECTION_RODATA ? IDX_RODATA
                     : us->section == SECTION_DATA ? IDX_DATA : IDX_BSS;
      uint8_t info = (STB_GLOBAL << 4) |
                     (us->section == SECTION_TEXT ? STT_FUNC : STT_NOTYPE);
      if (c64)
        elf_addsym64(sn, info, shn, us->off);
      else
        elf_addsym32(sn, info, shn, (uint32_t)us->off);
    }
  }
  for (int i = 0; i < ctx->nrelocs; i++) {
    rel *r = &ctx->relocs[i];
    int type = elf_reltype(arch, r->type);
    if (type < 0) {
      fprintf(stderr, "masm: elf: reloc class %u not supported on %s\n",
              r->type, arch);
      exit(1);
    }
    uint32_t sym = elf_sym_index(ctx, r);
    if (!c64 && r32rela) {
      uint8_t *b = elf_rel + elf_rel_used;
      uint32_t info = (sym << 8) | (uint32_t)type;
      b[0] = (uint8_t)r->adrs;
      b[1] = (uint8_t)(r->adrs >> 8);
      b[2] = (uint8_t)(r->adrs >> 16);
      b[3] = (uint8_t)(r->adrs >> 24);
      b[4] = (uint8_t)info;
      b[5] = (uint8_t)(info >> 8);
      b[6] = (uint8_t)(info >> 16);
      b[7] = (uint8_t)(info >> 24);
      b[8] = (uint8_t)r->addend;
      b[9] = (uint8_t)(r->addend >> 8);
      b[10] = (uint8_t)(r->addend >> 16);
      b[11] = (uint8_t)(r->addend >> 24);
      elf_rel_used += 12;
    } else if (!c64) {

      uint8_t *sec = elf_text;
      elf_patch(arch, sec, r);
      uint8_t *b = elf_rel + elf_rel_used;
      uint32_t info = (sym << 8) | (uint32_t)type;
      b[0] = (uint8_t)r->adrs;
      b[1] = (uint8_t)(r->adrs >> 8);
      b[2] = (uint8_t)(r->adrs >> 16);
      b[3] = (uint8_t)(r->adrs >> 24);
      b[4] = (uint8_t)info;
      b[5] = (uint8_t)(info >> 8);
      b[6] = (uint8_t)(info >> 16);
      b[7] = (uint8_t)(info >> 24);
      elf_rel_used += 8;
    } else {
      uint64_t info = ((uint64_t)sym << 32) | (uint64_t)type;
      uint64_t add = (uint64_t)r->addend;
      uint8_t *b = elf_rel + elf_rel_used;
      for (int k = 0; k < 8; k++) {
        b[k] = (uint8_t)(r->adrs >> (8 * k));
        b[8 + k] = (uint8_t)(info >> (8 * k));
        b[16 + k] = (uint8_t)(add >> (8 * k));
      }
      elf_rel_used += 24;
    }
  }
  int has_rel = elf_rel_used > 0;
  int nsh = has_rel ? 9 : 8;
  uint32_t align = c64 ? 8 : 4;
  uint64_t off = c64 ? 64 : 52;
  uint64_t text_off = off;
  off += (uint64_t)text_sz;
  uint64_t ro_off = off;
  off += (uint64_t)ro_sz;
  uint64_t da_off = off;
  off += (uint64_t)da_sz;
  uint64_t bss_off = off;
  off = (off + align - 1) & ~(uint64_t)(align - 1);
  uint64_t sym_off = off;
  off += elf_sym_used;
  uint64_t str_off = off;
  off += elf_str_used;
  uint64_t shs_off = off;
  off += elf_shstr_used;
  uint64_t rel_off = 0;
  if (has_rel) {
    off = (off + align - 1) & ~(uint64_t)(align - 1);
    rel_off = off;
    off += elf_rel_used;
  }
  off = (off + align - 1) & ~(uint64_t)(align - 1);
  uint64_t shoff = off;
  FILE *o = fopen(output_file, "wb");
  if (!o) {
    fprintf(stderr, "masm: cannot write %s\n", output_file);
    exit(1);
  }
  uint8_t ident[16] = {0x7F, 'E', 'L', 'F', c64 ? 2 : 1, 1, 1, 0};
  elf_wr(o, ident, 16);
  elf_w16(o, ET_REL);
  elf_w16(o, (uint32_t)elf_machine(arch));
  elf_w32(o, 1);
  if (c64) {
    elf_w64(o, 0);
    elf_w64(o, 0);
    elf_w64(o, shoff);
    elf_w32(o, elf_flags(arch));
    elf_w16(o, 64);
    elf_w16(o, 0);
    elf_w16(o, 0);
    elf_w16(o, 64);
    elf_w16(o, (uint32_t)nsh);
    elf_w16(o, IDX_SHSTRTAB);
  } else {
    elf_w32(o, 0);
    elf_w32(o, 0);
    elf_w32(o, (uint32_t)shoff);
    elf_w32(o, elf_flags(arch));
    elf_w16(o, 52);
    elf_w16(o, 0);
    elf_w16(o, 0);
    elf_w16(o, 40);
    elf_w16(o, (uint32_t)nsh);
    elf_w16(o, IDX_SHSTRTAB);
  }
  elf_wr(o, elf_text, (size_t)text_sz);
  elf_wr(o, elf_rodata, (size_t)ro_sz);
  elf_wr(o, elf_data, (size_t)da_sz);
  elf_pad(o, (long)sym_off);
  elf_wr(o, elf_sym, elf_sym_used);
  elf_wr(o, elf_str, elf_str_used);
  elf_wr(o, elf_shstr, elf_shstr_used);
  if (has_rel) {
    elf_pad(o, (long)rel_off);
    elf_wr(o, elf_rel, elf_rel_used);
  }
  elf_pad(o, (long)shoff);

#define SHDR32(name, typ, flags, offset, size, link, info, ent, align)          \
  do {                                                                         \
    elf_w32(o, (uint32_t)(name));                                              \
    elf_w32(o, (uint32_t)(typ));                                               \
    elf_w32(o, (uint32_t)(flags));                                             \
    elf_w32(o, 0);                                                             \
    elf_w32(o, (uint32_t)(offset));                                            \
    elf_w32(o, (uint32_t)(size));                                              \
    elf_w32(o, (uint32_t)(link));                                              \
    elf_w32(o, (uint32_t)(info));                                              \
    elf_w32(o, (uint32_t)(align));                                             \
    elf_w32(o, (uint32_t)(ent));                                               \
  } while (0)
#define SHDR64(name, typ, flags, offset, size, link, info, ent, align)          \
  do {                                                                         \
    elf_w32(o, (uint32_t)(name));                                              \
    elf_w32(o, (uint32_t)(typ));                                               \
    elf_w64(o, (uint64_t)(flags));                                             \
    elf_w64(o, 0);                                                             \
    elf_w64(o, (uint64_t)(offset));                                            \
    elf_w64(o, (uint64_t)(size));                                              \
    elf_w32(o, (uint32_t)(link));                                              \
    elf_w32(o, (uint32_t)(info));                                              \
    elf_w64(o, (uint64_t)(align));                                             \
    elf_w64(o, (uint64_t)(ent));                                               \
  } while (0)
#define SHDR(name, typ, flags, offset, size, link, info, ent, align)            \
  do {                                                                         \
    if (c64)                                                                   \
      SHDR64(name, typ, flags, offset, size, link, info, ent, align);           \
    else                                                                       \
      SHDR32(name, typ, flags, offset, size, link, info, ent, align);           \
  } while (0)
  SHDR(0, SHT_NULL, 0, 0, 0, 0, 0, 0, 0);
  SHDR(n_text, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, text_off,
       (uint64_t)text_sz, 0, 0, 0, c64 ? 16 : 4);
  SHDR(n_ro, SHT_PROGBITS, SHF_ALLOC, ro_off, (uint64_t)ro_sz, 0, 0, 0, 1);
  SHDR(n_da, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, da_off, (uint64_t)da_sz, 0, 0,
       0, 1);
  SHDR(n_bss, SHT_NOBITS, SHF_ALLOC | SHF_WRITE, bss_off, bss_sz, 0, 0, 0,
       c64 ? 16 : 4);
  SHDR(n_sym, SHT_SYMTAB, 0, sym_off, elf_sym_used, IDX_STRTAB, first_global,
       c64 ? 24 : 16, c64 ? 8 : 4);
  SHDR(n_str, SHT_STRTAB, 0, str_off, elf_str_used, 0, 0, 0, 1);
  SHDR(n_shs, SHT_STRTAB, 0, shs_off, elf_shstr_used, 0, 0, 0, 1);
  if (has_rel)
    SHDR(n_rel, rela ? SHT_RELA : SHT_REL, SHF_INFO_LINK, rel_off,
         elf_rel_used, IDX_SYMTAB, IDX_TEXT, c64 ? 24 : (r32rela ? 12 : 8),
         c64 ? 8 : 4);
#undef SHDR
#undef SHDR32
#undef SHDR64

  fclose(o);
  free(elf_sym);
  free(elf_rel);
  fprintf(stderr, "[+] elf%s: wrote %s (%ld text, %ld rodata, %d relocs)\n",
          c64 ? "64" : "32", output_file, text_sz, ro_sz, ctx->nrelocs);
}

static registry_object elf64_object = {
    REGISTRY_LINK, "elf64", NULL, NULL, elf_link,
};
static registry_object elf32_object = {
    REGISTRY_LINK, "elf32", NULL, NULL, elf_link,
};

static void elf_register(void) __attribute__((constructor));
static void elf_register(void) {
  register_object(&elf64_object);
  register_object(&elf32_object);
}

#endif
