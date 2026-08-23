#ifndef MASM_WASM_FORMAT_H
#define MASM_WASM_FORMAT_H

#include "../core.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void wasm_finish(context *ctx);

static uint8_t wf_body[1 << 18];

static void wf_u8(FILE *o, uint32_t v) { fputc((int)(v & 0xFF), o); }
static void wf_uleb(FILE *o, uint32_t v) {
  do {
    uint8_t b = (uint8_t)(v & 0x7F);
    v >>= 7;
    if (v)
      b |= 0x80;
    wf_u8(o, b);
  } while (v);
}

static void wf_section(FILE *o, uint8_t id, const uint8_t *p, uint32_t n) {
  wf_u8(o, id);
  wf_uleb(o, n);
  if (n && fwrite(p, 1, n, o) != n) {
    fprintf(stderr, "masm: wasm: write failed\n");
    exit(1);
  }
}

static void wasmobj_link(context *ctx, char *output_file) {
  const char *arch = ctx->arch ? ctx->arch : "";
  if (strcmp(arch, "wasm32") != 0 && strcmp(arch, "wasm64") != 0) {
    fprintf(stderr, "masm: wasm: arch '%s' not supported\n", arch);
    exit(1);
  }
  if (ctx->rodata_stream) {
    fflush(ctx->rodata_stream);
    if (ftell(ctx->rodata_stream) > 0)
      fprintf(stderr, "masm: wasm: warning: data sections ignored in v1\n");
  }
  if (ctx->data_stream) {
    fflush(ctx->data_stream);
    if (ftell(ctx->data_stream) > 0)
      fprintf(stderr, "masm: wasm: warning: data sections ignored in v1\n");
  }
  wasm_finish(ctx);
  long n = 0;
  if (ctx->text_stream) {
    fflush(ctx->text_stream);
    n = ftell(ctx->text_stream);
    if (n < 0)
      n = 0;
    if (n > (long)sizeof wf_body) {
      fprintf(stderr, "masm: wasm: code too large\n");
      exit(1);
    }
    rewind(ctx->text_stream);
    if (n && fread(wf_body, 1, (size_t)n, ctx->text_stream) != (size_t)n) {
      fprintf(stderr, "masm: wasm: read failed\n");
      exit(1);
    }
  }
  FILE *o = fopen(output_file, "wb");
  if (!o) {
    fprintf(stderr, "masm: cannot write %s\n", output_file);
    exit(1);
  }
  wf_u8(o, 0x00);
  wf_u8(o, 0x61);
  wf_u8(o, 0x73);
  wf_u8(o, 0x6D);
  wf_u8(o, 0x01);
  wf_u8(o, 0x00);
  wf_u8(o, 0x00);
  wf_u8(o, 0x00);
  uint8_t type_sec[] = {0x01, 0x60, 0x00, 0x00};
  wf_section(o, 1, type_sec, sizeof type_sec);
  uint8_t func_sec[] = {0x01, 0x00};
  wf_section(o, 3, func_sec, sizeof func_sec);
  uint8_t export_sec[] = {0x01, 0x04, 'm', 'a', 'i', 'n', 0x00, 0x00};
  wf_section(o, 7, export_sec, sizeof export_sec);
  wf_u8(o, 10);
  wf_uleb(o, (uint32_t)(n + 1));
  wf_uleb(o, 0x01);
  if (n && fwrite(wf_body, 1, (size_t)n, o) != (size_t)n) {
    fprintf(stderr, "masm: wasm: write failed\n");
    exit(1);
  }
  fclose(o);
  fprintf(stderr, "[+] wasm: wrote %s (%ld code bytes)\n", output_file, n);
}

static registry_object wasmobj_object = {
    REGISTRY_LINK, "wasm", NULL, NULL, wasmobj_link,
};

static void wasmobj_register(void) __attribute__((constructor));
static void wasmobj_register(void) { register_object(&wasmobj_object); }

#endif
