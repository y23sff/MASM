#define CORE_IMPLEMENTATION
#include "arch/arm.h"
#include "arch/arm64.h"
#include "arch/riscv.h"
#include "arch/wasm.h"
#include "arch/x86.h"
#include "core.h"
#include "format/coff.h"
#include "format/elf.h"
#include "format/wasm.h"
#include "parser/X26.h"
#include "parser/X26A.h"

#include <stdio.h>
#include <string.h>

static uint8_t text_mem[1 << 21];
static uint8_t rodata_mem[1 << 21];
static uint8_t data_mem[1 << 20];

static const char *default_format(const char *arch) {
  if (strcmp(arch, "wasm32") == 0 || strcmp(arch, "wasm64") == 0)
    return "wasm";
  if (strcmp(arch, "x86") == 0 || strcmp(arch, "arm") == 0 ||
      strcmp(arch, "rv32gc") == 0)
    return "elf32";
  return "elf64";
}

int main(int argc, char **argv) {
  const char *in = NULL;
  const char *out = "a.o";
  const char *arch = "x86_64";
  const char *fmt = NULL;
  const char *parser = "X26";
  int opt = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
      out = argv[++i];
    else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc)
      arch = argv[++i];
    else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
      fmt = argv[++i];
    else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
      parser = argv[++i];
    else if (strcmp(argv[i], "-O") == 0)
      opt = 1;
    else if (argv[i][0] != '-')
      in = argv[i];
    else {
      fputs("usage: masm [-a arch] [-f format] [-p parser] [-o out] input.s\n"
            "  arch:   x86_64 x86 arm arm64 rv32gc rv64gc wasm32 wasm64\n"
            "  format: elf64 elf32 coff wasm (default: by arch)\n"
            "  -O: x26a optimizer (dead code, nop runs to loops)\n",
            stderr);
      return 2;
    }
  }
  if (!in) {
    fputs("usage: masm [-a arch] [-f format] [-p parser] [-o out] input.s\n",
          stderr);
    return 2;
  }
  if (!fmt)
    fmt = default_format(arch);
  registry_object *p = find_registry((char *)parser);
  registry_object *e = find_registry((char *)arch);
  registry_object *l = find_registry((char *)fmt);
  if (!p || !p->parser) {
    fprintf(stderr, "masm: parser '%s' not registered\n", parser);
    return 1;
  }
  if (!e || !e->encoder) {
    fprintf(stderr, "masm: arch '%s' not registered\n", arch);
    return 1;
  }
  if (!l || !l->linker) {
    fprintf(stderr, "masm: format '%s' not registered\n", fmt);
    return 1;
  }
  fprintf(stderr, "[+] arch registered %s\n", e->name);
  fprintf(stderr, "[+] format registered %s\n", l->name);
  fprintf(stderr, "[+] parser registered %s\n", p->name);
  uint64_t bss = 0, text_adrs = 0;
  context ctx;
  memset(&ctx, 0, sizeof ctx);
  ctx.arch = arch;
  ctx.format = fmt;
  ctx.optimize = opt;
  ctx.text_stream = fmemopen(text_mem, sizeof text_mem, "w+b");
  ctx.rodata_stream = fmemopen(rodata_mem, sizeof rodata_mem, "w+b");
  ctx.data_stream = fmemopen(data_mem, sizeof data_mem, "w+b");
  if (!ctx.text_stream || !ctx.rodata_stream || !ctx.data_stream) {
    fputs("masm: fmemopen failed\n", stderr);
    return 1;
  }
  ctx.bss_size = &bss;
  ctx.current_text_adrs = &text_adrs;
  p->parser(&ctx, (char *)in);
  l->linker(&ctx, (char *)out);
  fclose(ctx.text_stream);
  fclose(ctx.rodata_stream);
  fclose(ctx.data_stream);
  free(ctx.relocs);
  free(ctx.syms);
  return 0;
}
