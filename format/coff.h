#ifndef MASM_COFF_H
#define MASM_COFF_H

#include "../core.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COFF_MACHINE_I386   0x014C
#define COFF_MACHINE_AMD64  0x8664
#define COFF_MACHINE_ARM    0x01C0
#define COFF_MACHINE_ARM64  0xAA64

#define IMAGE_REL_I386_DIR32    0x0006
#define IMAGE_REL_I386_REL32    0x0014
#define IMAGE_REL_AMD64_REL32   0x0004
#define IMAGE_REL_ARM_ADDR32    0x0001
#define IMAGE_REL_ARM_BRANCH24  0x0003
#define IMAGE_REL_ARM64_ADDR32  0x0001
#define IMAGE_REL_ARM64_BRANCH26 0x0003
#define IMAGE_REL_ARM64_REL21   0x0005
#define IMAGE_REL_ARM64_BRANCH19 0x000F

#define COFF_SYM_NULL      0
#define COFF_SYM_EXTERNAL  2
#define COFF_SYM_STATIC    3
#define COFF_TYPE_FUNCTION 0x20

#define COFF_SEC_CNT_CODE 0x00000020
#define COFF_SEC_INIT_DATA 0x00000040
#define COFF_SEC_UNINIT_DATA 0x00000080
#define COFF_SEC_MEM_READ 0x40000000
#define COFF_SEC_MEM_WRITE 0x80000000
#define COFF_SEC_MEM_EXECUTE 0x20000000

static uint8_t coff_text[1 << 18];
static uint8_t coff_rodata[1 << 18];
static uint8_t coff_data[1 << 16];

static int coff_machine(const char *a) {
  if (strcmp(a, "x86_64") == 0)
    return COFF_MACHINE_AMD64;
  if (strcmp(a, "x86") == 0)
    return COFF_MACHINE_I386;
  if (strcmp(a, "arm") == 0)
    return COFF_MACHINE_ARM;
  if (strcmp(a, "arm64") == 0)
    return COFF_MACHINE_ARM64;
  return -1;
}

static int coff_reltype(const char *a, uint8_t t) {
  if (strcmp(a, "x86_64") == 0)
    return t == REL_PC32 ? IMAGE_REL_AMD64_REL32 : -1;
  if (strcmp(a, "x86") == 0)
    return t == REL_ABS32 ? IMAGE_REL_I386_DIR32
           : t == REL_PC32 ? IMAGE_REL_I386_REL32 : -1;
  if (strcmp(a, "arm") == 0)
    return t == REL_ABS32 ? IMAGE_REL_ARM_ADDR32
           : (t == REL_BRJMP || t == REL_BRCALL) ? IMAGE_REL_ARM_BRANCH24 : -1;
  if (t == REL_ABS32)
    return IMAGE_REL_ARM64_ADDR32;
  if (t == REL_BRJMP || t == REL_BRCALL)
    return IMAGE_REL_ARM64_BRANCH26;
  if (t == REL_ADR21)
    return IMAGE_REL_ARM64_REL21;
  if (t == REL_BCC19)
    return IMAGE_REL_ARM64_BRANCH19;
  return -1;
}

static long coff_slurp(FILE *f, uint8_t *dst, long cap) {
  if (!f)
    return 0;
  fflush(f);
  long sz = ftell(f);
  if (sz < 0)
    sz = 0;
  if (sz > cap) {
    fprintf(stderr, "masm: coff: section too large\n");
    exit(1);
  }
  rewind(f);
  if (sz && fread(dst, 1, (size_t)sz, f) != (size_t)sz) {
    fprintf(stderr, "masm: coff: read failed\n");
    exit(1);
  }
  return sz;
}

static uint32_t coff_r32(const uint8_t *b, uint64_t at) {
  return (uint32_t)b[at] | ((uint32_t)b[at + 1] << 8) |
         ((uint32_t)b[at + 2] << 16) | ((uint32_t)b[at + 3] << 24);
}
static void coff_w32b(uint8_t *b, uint64_t at, uint32_t v) {
  b[at] = (uint8_t)v;
  b[at + 1] = (uint8_t)(v >> 8);
  b[at + 2] = (uint8_t)(v >> 16);
  b[at + 3] = (uint8_t)(v >> 24);
}

static void coff_embed(const char *arch, uint8_t *buf, rel *r) {
  int64_t a = r->addend;
  if (strcmp(arch, "arm") == 0 &&
      (r->type == REL_BRJMP || r->type == REL_BRCALL)) {
    uint32_t w = coff_r32(buf, r->adrs);
    w = (w & 0xFF000000u) | ((uint32_t)(a >> 2) & 0x00FFFFFFu);
    coff_w32b(buf, r->adrs, w);
    return;
  }
  if (strcmp(arch, "arm64") == 0 &&
      (r->type == REL_BRJMP || r->type == REL_BRCALL)) {
    uint32_t w = coff_r32(buf, r->adrs);
    w = (w & 0xFC000000u) | ((uint32_t)(a >> 2) & 0x03FFFFFFu);
    coff_w32b(buf, r->adrs, w);
    return;
  }
  if (strcmp(arch, "arm64") == 0 && r->type == REL_ADR21) {

    uint32_t w = coff_r32(buf, r->adrs);
    uint32_t imm = (uint32_t)a & 0x1FFFFFu;
    w = (w & 0x9F00001Fu) | ((imm & 3u) << 29) | (((imm >> 2) & 0x7FFFFu) << 5);
    coff_w32b(buf, r->adrs, w);
    return;
  }
  coff_w32b(buf, r->adrs, (uint32_t)(int32_t)a);
}

static uint32_t coff_secsym(SECTION s) {
  switch (s) {
  case SECTION_TEXT:
    return 1;
  case SECTION_RODATA:
    return 3;
  case SECTION_DATA:
    return 5;
  case SECTION_BSS:
    return 7;
  default:
    return 0;
  }
}

static uint32_t coff_sym_index(context *ctx, rel *r) {
  if (r->section != SECTION_NULL || !r->name)
    return coff_secsym(r->section);
  for (int i = 0; i < ctx->nsyms; i++)
    if (ctx->syms[i].kind == SYM_EXTERN &&
        strcmp(ctx->syms[i].name, r->name) == 0)
      return (ctx->nsyms == 0 ? 10u : 9u) + (uint32_t)i;
  fprintf(stderr, "masm: coff: extern symbol '%s' not declared\n", r->name);
  exit(1);
}

static void coff_w8(FILE *o, uint32_t v) { fputc((int)(v & 0xFF), o); }
static void coff_w16(FILE *o, uint32_t v) {
  coff_w8(o, v);
  coff_w8(o, v >> 8);
}
static void coff_w32(FILE *o, uint32_t v) {
  coff_w16(o, v);
  coff_w16(o, v >> 16);
}

static void coff_link(context *ctx, char *output_file) {
  const char *arch = ctx->arch ? ctx->arch : "x86_64";
  int machine = coff_machine(arch);
  if (machine < 0) {
    fprintf(stderr, "masm: coff: arch '%s' not supported\n", arch);
    exit(1);
  }
  long text_sz = coff_slurp(ctx->text_stream, coff_text, sizeof coff_text);
  long ro_sz = coff_slurp(ctx->rodata_stream, coff_rodata, sizeof coff_rodata);
  long da_sz = coff_slurp(ctx->data_stream, coff_data, sizeof coff_data);
  uint64_t bss_sz = ctx->bss_size ? *ctx->bss_size : 0;
  if (bss_sz > 0xFFFFFFFFull) {
    fprintf(stderr, "masm: coff: bss too large\n");
    exit(1);
  }
  int nrel = 0;
  for (int i = 0; i < ctx->nrelocs; i++) {
    rel *r = &ctx->relocs[i];
    if (coff_reltype(arch, r->type) < 0) {
      fprintf(stderr, "masm: coff: reloc class %u not supported on %s\n",
              r->type, arch);
      exit(1);
    }
    coff_embed(arch, coff_text, r);
    nrel++;
  }
  uint32_t headers = 20 + 4 * 40;
  uint64_t text_off = headers;
  uint64_t ro_off = text_off + (uint64_t)text_sz;
  uint64_t da_off = ro_off + (uint64_t)ro_sz;
  uint64_t rel_off = da_off + (uint64_t)da_sz;
  rel_off = (rel_off + 1) & ~(uint64_t)1;
  uint64_t rel_bytes = (uint64_t)nrel * 10;
  static uint8_t coff_strt[8192];
  uint32_t strt_used = 4;
  uint32_t *coff_stroff =
      (uint32_t *)malloc(sizeof(uint32_t) * (size_t)(ctx->nsyms + 1));
  if (!coff_stroff) {
    fprintf(stderr, "masm: coff: out of memory\n");
    exit(1);
  }
  for (int i = 0; i < ctx->nsyms && i < 256; i++) {
    size_t l = strlen(ctx->syms[i].name);
    coff_stroff[i] = 0;
    if (l > 8) {
      if (strt_used + l + 1 > sizeof coff_strt) {
        fprintf(stderr, "masm: coff: string table full\n");
        exit(1);
      }
      coff_stroff[i] = strt_used;
      memcpy(coff_strt + strt_used, ctx->syms[i].name, l + 1);
      strt_used += (uint32_t)(l + 1);
    }
  }

  uint64_t sym_off = rel_off + rel_bytes;
  sym_off = (sym_off + 3) & ~(uint64_t)3;
  FILE *o = fopen(output_file, "wb");
  if (!o) {
    fprintf(stderr, "masm: cannot write %s\n", output_file);
    exit(1);
  }
  coff_w16(o, (uint32_t)machine);
  coff_w16(o, 4);
  coff_w32(o, 0);
  coff_w32(o, (uint32_t)sym_off);
  coff_w32(o, (uint32_t)((ctx->nsyms ? 9 : 10) + ctx->nsyms));
  coff_w16(o, 0);
  coff_w16(o, 0);
  static const char *names[4] = {".text", ".rdata", ".data", ".bss"};
  for (int i = 0; i < 4; i++) {
    char nm[8] = {0};
    memcpy(nm, names[i], strlen(names[i]));
    fwrite(nm, 1, 8, o);
  uint32_t flags = i == 0
                       ? COFF_SEC_CNT_CODE | COFF_SEC_MEM_EXECUTE |
                             COFF_SEC_MEM_READ
                   : i == 1 ? COFF_SEC_CNT_CODE | COFF_SEC_MEM_READ
                   : i == 2 ? COFF_SEC_INIT_DATA | COFF_SEC_MEM_READ |
                             COFF_SEC_MEM_WRITE
                            : COFF_SEC_UNINIT_DATA | COFF_SEC_MEM_READ |
                                  COFF_SEC_MEM_WRITE;
    long sz = i == 0 ? text_sz : i == 1 ? ro_sz : da_sz;
    uint64_t vsz = i == 3 ? bss_sz : (uint64_t)sz;
    uint64_t raw = i == 3 ? 0 : (uint64_t)sz;
    uint64_t off = i == 0 ? text_off : i == 1 ? ro_off : da_off;
    if (i == 3)
      off = da_off + (uint64_t)da_sz;
    coff_w32(o, (uint32_t)vsz);
    coff_w32(o, 0);
    coff_w32(o, (uint32_t)raw);
    coff_w32(o, (uint32_t)(raw ? off : 0));
    coff_w32(o, (uint32_t)(i == 0 && nrel ? rel_off : 0));
    coff_w32(o, 0);
    coff_w16(o, (uint32_t)(i == 0 ? nrel : 0));
    coff_w16(o, 0);
    coff_w32(o, flags);
  }
  fwrite(coff_text, 1, (size_t)text_sz, o);
  fwrite(coff_rodata, 1, (size_t)ro_sz, o);
  fwrite(coff_data, 1, (size_t)da_sz, o);
  while ((uint64_t)ftell(o) < rel_off)
    fputc(0, o);
  for (int i = 0; i < ctx->nrelocs; i++) {
    rel *r = &ctx->relocs[i];
    coff_w32(o, (uint32_t)r->adrs);
    coff_w32(o, coff_sym_index(ctx, r));
    coff_w16(o, (uint32_t)coff_reltype(arch, r->type));
  }
  while ((uint64_t)ftell(o) < sym_off)
    fputc(0, o);
  static const uint8_t sym_pad[18] = {0};
  fwrite(sym_pad, 1, 18, o);
  for (int i = 0; i < 4; i++) {
    char nm[8] = {0};
    memcpy(nm, names[i], strlen(names[i]));
    fwrite(nm, 1, 8, o);
    coff_w32(o, 0);
    coff_w16(o, 0);
    coff_w16(o, 0);
    fputc(COFF_SYM_STATIC, o);
    fputc(0, o);
    coff_w16(o, (uint32_t)(i + 1));
    long sz = i == 0 ? text_sz : i == 1 ? ro_sz : da_sz;
    uint64_t vsz = i == 3 ? bss_sz : (uint64_t)sz;
    coff_w32(o, (uint32_t)vsz);
    coff_w16(o, (uint32_t)(i == 0 ? nrel : 0));
    coff_w16(o, 0);
    coff_w32(o, 0);
    coff_w16(o, (uint32_t)(i + 1));
    coff_w16(o, 0);
    coff_w16(o, 0);
  }
  if (ctx->nsyms == 0) {
    fwrite("_start", 1, 7, o);
    coff_w32(o, ctx->has_start ? (uint32_t)ctx->start_off : 0);
    coff_w16(o, COFF_TYPE_FUNCTION);
    fputc(COFF_SYM_EXTERNAL, o);
    fputc(0, o);
    coff_w16(o, 1);
  }
  for (int i = 0; i < ctx->nsyms; i++) {
    sym *us = &ctx->syms[i];
    char nm[8] = {0};
    size_t l = strlen(us->name);
    if (l <= 8)
      memcpy(nm, us->name, l);
    else
      snprintf(nm, sizeof nm, "/%u", coff_stroff[i]);
    fwrite(nm, 1, 8, o);
    coff_w32(o, us->kind == SYM_EXPORT ? (uint32_t)us->off : 0);
    coff_w16(o, 0);
    coff_w16(o, us->kind == SYM_EXPORT ? COFF_TYPE_FUNCTION : 0);
    fputc(COFF_SYM_EXTERNAL, o);
    fputc(0, o);
    uint16_t secnum = 0;
    if (us->kind == SYM_EXPORT)
      secnum = us->section == SECTION_TEXT     ? 1
               : us->section == SECTION_RODATA ? 2
               : us->section == SECTION_DATA   ? 3
                                               : 4;
    coff_w16(o, secnum);
  }
  coff_w32(o, strt_used);
  if (strt_used > 4)
    fwrite(coff_strt + 4, 1, strt_used - 4, o);
  free(coff_stroff);
  fclose(o);
  fprintf(stderr, "[+] coff: wrote %s (%ld text, %ld rdata, %d relocs)\n",
          output_file, text_sz, ro_sz, nrel);
}

static registry_object coff_object = {
    REGISTRY_LINK, "coff", NULL, NULL, coff_link,
};

static void coff_register(void) __attribute__((constructor));
static void coff_register(void) { register_object(&coff_object); }

#endif
