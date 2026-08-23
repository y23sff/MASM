#ifndef MASM_CORE_H
#define MASM_CORE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { OPERAND_NULL, OPERAND_REG, OPERAND_IMM, OPERAND_REL } OPERAND;

typedef enum {
  REL_NONE = 0,
  REL_PC32,
  REL_ABS32,
  REL_BRJMP,
  REL_BRCALL,
  REL_BCC19,
  REL_ADR21,
  RV_HI20,
  RV_LO12,
  RV_JAL,
  RV_BRANCH
} RELK;

typedef enum {
  WIDTH_NULL = 0,
  WIDTH_BYTE,
  WIDTH_WORD,
  WIDTH_DWORD,
  WIDTH_QWORD
} WIDTH;

typedef enum {
  SECTION_NULL,
  SECTION_TEXT,
  SECTION_DATA,
  SECTION_BSS,
  SECTION_RODATA
} SECTION;

typedef enum {
  REGISTRY_PARSER,
  REGISTRY_ENCODER,
  REGISTRY_LINK,
  REGISTRY_KIT
} REGISTRY_KIND;

typedef struct {
  uint64_t adrs;
  SECTION section;
  int64_t addend;
  uint8_t type;
  const char *name;
} rel;

typedef enum { SYM_NULL, SYM_EXPORT, SYM_EXTERN } SYMK;

typedef struct {
  char name[64];
  uint8_t kind;
  SECTION section;
  uint64_t off;
} sym;

typedef struct {
  FILE *data_stream, *rodata_stream, *text_stream;
  uint64_t *bss_size, *current_text_adrs;
  rel *relocs;
  int nrelocs, arelocs;
  sym *syms;
  int nsyms, asyms;
  char *input_name;
  const char *arch;
  const char *format;
  uint64_t start_off;
  int has_start;
  int optimize;
} context;

typedef void (*parse)(context *ctx, char *input_file);
typedef void (*encode)(context *ctx, char *mnemonic, WIDTH width, OPERAND op_a,
                       uint64_t imm_a, rel rel_a, OPERAND op_b, uint64_t imm_b,
                       rel rel_b);
typedef void (*link)(context *ctx, char *output_file);

typedef struct {
  REGISTRY_KIND kind;
  char *name;
  parse parser;
  encode encoder;
  link linker;
} registry_object;

extern registry_object *registry[32];
extern int registry_index;

void register_object(registry_object *a);
registry_object *find_registry(char *name);
registry_object *find_registry_kind(REGISTRY_KIND kind);
rel *ctx_new_rel(context *ctx);
sym *ctx_new_sym(context *ctx);
void masm_dief(const char *who, const char *fmt, ...);
extern uint32_t masm_line;

#ifdef CORE_IMPLEMENTATION

registry_object *registry[32];
int registry_index = 0;

void register_object(registry_object *a) {
  if (registry_index >= 32) {
    fprintf(stderr, "masm: registry full\n");
    return;
  }
  registry[registry_index++] = a;
}

registry_object *find_registry(char *name) {
  for (int i = 0; i < registry_index; i++)
    if (strcmp(registry[i]->name, name) == 0)
      return registry[i];
  return NULL;
}

registry_object *find_registry_kind(REGISTRY_KIND kind) {
  for (int i = 0; i < registry_index; i++)
    if (registry[i]->kind == kind)
      return registry[i];
  return NULL;
}

uint32_t masm_line = 0;

static void *ctx_grow(void *p, int *cap, int need, size_t esz) {
  if (need < *cap)
    return p;
  int nc = *cap ? *cap * 2 : 64;
  while (nc < need)
    nc *= 2;
  p = realloc(p, (size_t)nc * esz);
  if (!p) {
    fprintf(stderr, "masm: out of memory\n");
    exit(1);
  }
  *cap = nc;
  return p;
}

rel *ctx_new_rel(context *ctx) {
  ctx->relocs = (rel *)ctx_grow(ctx->relocs, &ctx->arelocs, ctx->nrelocs + 1,
                                sizeof(rel));
  memset(&ctx->relocs[ctx->nrelocs], 0, sizeof(rel));
  return &ctx->relocs[ctx->nrelocs++];
}

sym *ctx_new_sym(context *ctx) {
  ctx->syms = (sym *)ctx_grow(ctx->syms, &ctx->asyms, ctx->nsyms + 1,
                              sizeof(sym));
  memset(&ctx->syms[ctx->nsyms], 0, sizeof(sym));
  return &ctx->syms[ctx->nsyms++];
}

void masm_dief(const char *who, const char *fmt, ...) {
  va_list ap;
  if (masm_line)
    fprintf(stderr, "masm: line %u: %s: ", masm_line, who);
  else
    fprintf(stderr, "masm: %s: ", who);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(1);
}

#endif
#endif
