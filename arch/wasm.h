#ifndef MASM_WASM_H
#define MASM_WASM_H

#include "../core.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WASM_BUF_MAX (1 << 18)
#define WASM_BLOCKS_MAX 1024
#define WASM_NAMES_MAX 1024
#define WASM_NAME_LEN 64

#define W_ANY 0xFFu

enum {
  W_NOP = 1,
  W_RET,
  W_ERR,
  W_MOV_RR,
  W_MOV_IR,
  W_ALU_RR,
  W_ALU_IR,
  W_CMP,
  W_TEST,
  W_BR,
  W_BCC
};

typedef struct {
  const char *mn;
  uint8_t oa, ob;
  uint8_t kind;
  uint8_t op32, op64;
} wasm_form;

#define F(mn, oa, ob, k, a, b)                                                 \
  { mn, (uint8_t)(oa), (uint8_t)(ob), (uint8_t)(k), (uint8_t)(a), (uint8_t)(b) }

static const wasm_form w_tab[] = {
    F("nop", OPERAND_NULL, OPERAND_NULL, W_NOP, 0, 0),
    F("ret", OPERAND_NULL, OPERAND_NULL, W_RET, 0, 0),
    F("syscall", OPERAND_NULL, OPERAND_NULL, W_ERR, 0, 0),
    F("int", OPERAND_IMM, OPERAND_NULL, W_ERR, 0, 0),
    F("call", W_ANY, W_ANY, W_ERR, 0, 0),
    F("push", OPERAND_REG, OPERAND_NULL, W_ERR, 0, 0),
    F("pop", OPERAND_REG, OPERAND_NULL, W_ERR, 0, 0),

    F("mov", OPERAND_REG, OPERAND_REG, W_MOV_RR, 0, 0),
    F("mov", OPERAND_REG, OPERAND_IMM, W_MOV_IR, 0, 0),
    F("cmp", OPERAND_REG, OPERAND_REG, W_CMP, 0, 0),
    F("test", OPERAND_REG, OPERAND_REG, W_TEST, 0, 0),

    F("add", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x6A, 0x7C),
    F("add", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x6A, 0x7C),
    F("sub", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x6B, 0x7D),
    F("sub", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x6B, 0x7D),
    F("mul", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x6C, 0x7E),
    F("mul", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x6C, 0x7E),
    F("imul", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x6C, 0x7E),
    F("imul", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x6C, 0x7E),
    F("div", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x6D, 0x7F),
    F("div", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x6D, 0x7F),
    F("idiv", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x6D, 0x7F),
    F("idiv", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x6D, 0x7F),
    F("and", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x71, 0x83),
    F("and", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x71, 0x83),
    F("or", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x72, 0x84),
    F("or", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x72, 0x84),
    F("xor", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x73, 0x85),
    F("xor", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x73, 0x85),
    F("shl", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x74, 0x86),
    F("shl", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x74, 0x86),
    F("shr", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x76, 0x88),
    F("shr", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x76, 0x88),
    F("sar", OPERAND_REG, OPERAND_REG, W_ALU_RR, 0x75, 0x87),
    F("sar", OPERAND_REG, OPERAND_IMM, W_ALU_IR, 0x75, 0x87),

    F("jmp", OPERAND_REL, OPERAND_NULL, W_BR, 0, 0),
    F("b", OPERAND_REL, OPERAND_NULL, W_BR, 0, 0),

    F("je", OPERAND_REL, OPERAND_NULL, W_BCC, 0x46, 0x51),
    F("jz", OPERAND_REL, OPERAND_NULL, W_BCC, 0x46, 0x51),
    F("jne", OPERAND_REL, OPERAND_NULL, W_BCC, 0x47, 0x52),
    F("jnz", OPERAND_REL, OPERAND_NULL, W_BCC, 0x47, 0x52),
    F("jb", OPERAND_REL, OPERAND_NULL, W_BCC, 0x49, 0x54),
    F("jc", OPERAND_REL, OPERAND_NULL, W_BCC, 0x49, 0x54),
    F("jnae", OPERAND_REL, OPERAND_NULL, W_BCC, 0x49, 0x54),
    F("jae", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4F, 0x59),
    F("jnb", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4F, 0x59),
    F("jnc", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4F, 0x59),
    F("jbe", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4D, 0x58),
    F("jna", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4D, 0x58),
    F("ja", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4B, 0x56),
    F("jnbe", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4B, 0x56),
    F("js", OPERAND_REL, OPERAND_NULL, W_BCC, 0x48, 0x53),
    F("jns", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4E, 0x59),
    F("jl", OPERAND_REL, OPERAND_NULL, W_BCC, 0x48, 0x53),
    F("jnge", OPERAND_REL, OPERAND_NULL, W_BCC, 0x48, 0x53),
    F("jge", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4E, 0x59),
    F("jnl", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4E, 0x59),
    F("jle", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4C, 0x57),
    F("jng", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4C, 0x57),
    F("jg", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4A, 0x55),
    F("jnle", OPERAND_REL, OPERAND_NULL, W_BCC, 0x4A, 0x55),
};

#undef F

static uint8_t wasm_buf[WASM_BUF_MAX];
static int wasm_len;
typedef struct {
  int id;
  int is_loop;
  uint64_t endpos;
} wblock;
static wblock w_stack[WASM_BLOCKS_MAX];
static int w_nstack;
static int wasm_cmp_a, wasm_cmp_b, wasm_cmp_kind;
static int wasm_mode64;
static uint64_t wasm_last = 1;

static char wasm_names[WASM_NAMES_MAX][WASM_NAME_LEN];
static int wasm_nnames;
static int wasm_ldef[WASM_NAMES_MAX];
static int wasm_isloop[WASM_NAMES_MAX];
static int wasm_backseen[WASM_NAMES_MAX];

static void wasm_die(const char *mn, const char *why) {
  fprintf(stderr, "masm: wasm: '%s' not supported (%s)\n", mn, why);
  exit(1);
}

static void wasm_put(uint8_t b) {
  if (wasm_len < WASM_BUF_MAX)
    wasm_buf[wasm_len] = b;
  wasm_len++;
}

static void wasm_u32p(uint32_t v) {
  for (int i = 0; i < 5; i++)
    wasm_put((uint8_t)(((v >> (7 * i)) & 0x7F) | (i < 4 ? 0x80 : 0)));
}

static void wasm_const(int64_t v) {
  int n = wasm_mode64 ? 10 : 5;
  wasm_put(wasm_mode64 ? 0x42 : 0x41);
  for (int i = 0; i < n; i++)
    wasm_put((i == n - 1)
                 ? (uint8_t)((v >> (7 * i)) & 0x7F)
                 : (uint8_t)(((v >> (7 * i)) & 0x7F) | 0x80));
}

static void wasm_get(uint32_t idx) {
  wasm_put(0x20);
  wasm_put((uint8_t)idx);
}

static void wasm_set(uint32_t idx) {
  wasm_put(0x21);
  wasm_put((uint8_t)idx);
}

static int wasm_order(const char *name) {
  for (int i = 0; i < wasm_nnames; i++)
    if (strncmp(wasm_names[i], name, WASM_NAME_LEN) == 0)
      return i;
  if (wasm_nnames >= WASM_NAMES_MAX || strlen(name) >= WASM_NAME_LEN) {
    fprintf(stderr, "masm: wasm: too many labels\n");
    exit(1);
  }
  snprintf(wasm_names[wasm_nnames], WASM_NAME_LEN, "%s", name);
  return wasm_nnames++;
}

static int w_match(uint8_t want, OPERAND got) {
  if (want == W_ANY)
    return 1;
  return want == (uint8_t)got;
}

static const wasm_form *w_find(const char *mn, OPERAND oa, OPERAND ob) {
  for (size_t i = 0; i < sizeof w_tab / sizeof w_tab[0]; i++) {
    const wasm_form *f = &w_tab[i];
    if (strcmp(f->mn, mn) != 0)
      continue;
    if (w_match(f->oa, oa) && w_match(f->ob, ob))
      return f;
  }
  return NULL;
}

static uint8_t w_reg(uint64_t idx, const char *mn) {
  if (idx == 9 || idx == 10)
    wasm_die(mn, "wasm has no sp/fp registers");
  return (uint8_t)(idx & 15);
}

static int w_is_shift(const wasm_form *f) {
  return f->op32 == 0x74 || f->op32 == 0x75 || f->op32 == 0x76;
}

static void wasm_encode_mode(context *ctx, char *mnemonic, WIDTH width,
                             OPERAND oa, uint64_t ia, rel ra, OPERAND ob,
                             uint64_t ib, rel rb, int mode64) {
  const char *mn = mnemonic;

  if (mn[0] == '.' && strncmp(mn, ".lbl ", 5) == 0) {
    uint64_t now0 = *ctx->current_text_adrs;
    if (now0 == 0 && wasm_last != 0) {
      wasm_len = 0;
      w_nstack = 0;
      for (int i = 0; i < WASM_NAMES_MAX; i++) {
        wasm_ldef[i] = 0;
        wasm_backseen[i] = 0;
      }
    }
    int ord = wasm_order(mn + 5);
    int was = wasm_len;
    while (w_nstack && !w_stack[w_nstack - 1].is_loop &&
           w_stack[w_nstack - 1].id == ord)
      wasm_put(0x0B), w_nstack--;
    while (w_nstack && w_stack[w_nstack - 1].is_loop &&
           wasm_backseen[w_stack[w_nstack - 1].id])
      wasm_put(0x0B), w_nstack--;
    for (int i = 0; i < w_nstack; i++) {
      if (!w_stack[i].is_loop)
        continue;
      for (int j = i + 1; j < w_nstack; j++)
        if (!w_stack[j].is_loop) {
          fprintf(stderr, "masm: wasm: unstructured region at '%s'\n",
                  mn + 5);
          exit(1);
        }
    }
    if (wasm_isloop[ord]) {
      wasm_put(0x03);
      wasm_put(0x40);
      w_stack[w_nstack].id = ord;
      w_stack[w_nstack].is_loop = 1;
      w_stack[w_nstack].endpos = 0;
      w_nstack++;
    }
    wasm_ldef[ord] = 1;
    *ctx->current_text_adrs += (uint64_t)(wasm_len - was);
    wasm_last = *ctx->current_text_adrs;
    return;
  }

  wasm_mode64 = mode64;

  if (*ctx->current_text_adrs == 0 && wasm_last != 0) {
    wasm_len = 0;
    w_nstack = 0;
    wasm_cmp_kind = 0;
    for (int i = 0; i < WASM_NAMES_MAX; i++) {
      wasm_ldef[i] = 0;
      wasm_backseen[i] = 0;
    }
  }

  if (mode64) {
    if (width == WIDTH_WORD || width == WIDTH_BYTE)
      wasm_die(mn, "word/byte width unsupported");
  } else {
    if (width == WIDTH_QWORD)
      wasm_die(mn, "qword needs wasm64");
    if (width == WIDTH_WORD || width == WIDTH_BYTE)
      wasm_die(mn, "word/byte width unsupported");
  }

  const wasm_form *f = w_find(mn, oa, ob);
  if (!f) {
    masm_dief("wasm", "no form for '%s'", mn);
  }

  int start = wasm_len;
  uint8_t rd = (uint8_t)(ia & 7);
  uint8_t rs = (uint8_t)(ib & 7);
  if (oa == OPERAND_REG)
    rd = w_reg(ia, mn);
  if (ob == OPERAND_REG)
    rs = w_reg(ib, mn);

  switch (f->kind) {
  case W_NOP:
    wasm_put(0x01);
    break;
  case W_RET:
    wasm_put(0x0F);
    break;
  case W_ERR:
    wasm_die(mn, "not in wasm v1");
    break;
  case W_MOV_RR:
    wasm_get(rs);
    wasm_set(rd);
    break;
  case W_MOV_IR:
    wasm_const((int64_t)ib);
    wasm_set(rd);
    break;
  case W_ALU_RR:
    wasm_get(rd);
    wasm_get(rs);
    if (mode64 && w_is_shift(f))
      wasm_put(0xA7);
    wasm_put(mode64 ? f->op64 : f->op32);
    wasm_set(rd);
    break;
  case W_ALU_IR:
    wasm_get(rd);
    wasm_const((int64_t)ib);
    wasm_put(mode64 ? f->op64 : f->op32);
    wasm_set(rd);
    break;
  case W_CMP:
    wasm_cmp_a = (int)ia;
    wasm_cmp_b = (int)ib;
    wasm_cmp_kind = 1;
    break;
  case W_TEST:
    wasm_cmp_a = (int)ia;
    wasm_cmp_b = (int)ib;
    wasm_cmp_kind = 2;
    break;
  case W_BR:
  case W_BCC: {
    if (f->kind == W_BCC && wasm_cmp_kind == 0)
      wasm_die(mn, "conditional branch needs a preceding cmp/test");
    const char *name = ra.name ? ra.name : "";
    if (!name[0])
      wasm_die(mn, "branch needs a label");
    if (ra.section != SECTION_TEXT && ra.section != SECTION_NULL)
      wasm_die(mn, "branch target must be a .text label");

    int ord = wasm_order(name);
    int resolved = (ra.section == SECTION_TEXT);
    uint64_t tpos = resolved ? ra.adrs : 0;

    if (wasm_ldef[ord]) {
      wasm_isloop[ord] = 1;
      wasm_backseen[ord] = 1;
      if (f->kind == W_BCC) {
        int is_test = (wasm_cmp_kind == 2);
        int ne_form = is_test && (f->op32 == 0x47);
        wasm_get((uint32_t)wasm_cmp_a);
        wasm_get((uint32_t)wasm_cmp_b);
        if (is_test) {
          wasm_put(0x71);
          if (!ne_form)
            wasm_put(0x45);
        } else {
          wasm_put(mode64 ? f->op64 : f->op32);
        }
      }
      uint32_t depth = 0;
      int k = w_nstack - 1;
      while (k >= 0 && !(w_stack[k].is_loop && w_stack[k].id == ord))
        k--;
      if (k < 0) {
        if (resolved)
          wasm_die(name, "loop already closed (second back edge too late)");
        depth = 0;
      } else {
        depth = (uint32_t)(w_nstack - 1 - k);
      }
      wasm_put(f->kind == W_BCC ? 0x0D : 0x0C);
      wasm_u32p(depth);
      break;
    }

    int have = 0;
    for (int i = 0; i < w_nstack; i++)
      if (!w_stack[i].is_loop && w_stack[i].id == ord)
        have = 1;
    if (!have) {
      if (resolved)
        for (int i = 0; i < w_nstack; i++)
          if (!w_stack[i].is_loop && w_stack[i].endpos < tpos) {
            fprintf(stderr,
                    "masm: wasm: unstructured branch to '%s' (crosses block)\n",
                    name);
            exit(1);
          }
      if (w_nstack >= WASM_BLOCKS_MAX)
        wasm_die(mn, "too many open blocks");
      wasm_put(0x02);
      wasm_put(0x40);
      w_stack[w_nstack].id = ord;
      w_stack[w_nstack].is_loop = 0;
      w_stack[w_nstack].endpos = tpos;
      w_nstack++;
    } else {
      wasm_put(0x01);
      wasm_put(0x01);
    }
    if (f->kind == W_BCC) {
      int is_test = (wasm_cmp_kind == 2);
      int ne_form = is_test && (f->op32 == 0x47);
      wasm_get((uint32_t)wasm_cmp_a);
      wasm_get((uint32_t)wasm_cmp_b);
      if (is_test) {
        wasm_put(0x71);
        if (!ne_form)
          wasm_put(0x45);
      } else {
        wasm_put(mode64 ? f->op64 : f->op32);
      }
    }
    uint32_t depth = 0;
    if (resolved)
      for (int i = 0; i < w_nstack; i++)
        if (!w_stack[i].is_loop && w_stack[i].endpos < tpos)
          depth++;
    wasm_put(f->kind == W_BCC ? 0x0D : 0x0C);
    wasm_u32p(depth);
    break;
  }
  default:
    wasm_die(mn, "encode failed");
  }

  if (wasm_len > WASM_BUF_MAX) {
    fprintf(stderr, "masm: wasm: code too large\n");
    exit(1);
  }
  *ctx->current_text_adrs += (uint64_t)(wasm_len - start);
  wasm_last = *ctx->current_text_adrs;
}

static void wasm32_encode(context *ctx, char *mnemonic, WIDTH width,
                          OPERAND oa, uint64_t ia, rel ra, OPERAND ob,
                          uint64_t ib, rel rb) {
  wasm_encode_mode(ctx, mnemonic, width, oa, ia, ra, ob, ib, rb, 0);
}

static void wasm64_encode(context *ctx, char *mnemonic, WIDTH width,
                          OPERAND oa, uint64_t ia, rel ra, OPERAND ob,
                          uint64_t ib, rel rb) {
  wasm_encode_mode(ctx, mnemonic, width, oa, ia, ra, ob, ib, rb, 1);
}

void wasm_finish(context *ctx) {
  while (w_nstack)
    wasm_put(0x0B), w_nstack--;
  wasm_put(0x0B);
  if (!ctx->text_stream)
    return;
  uint32_t body = (uint32_t)wasm_len + 3;
  uint8_t leb[5];
  int n = 0;
  do {
    uint8_t b = (uint8_t)(body & 0x7F);
    body >>= 7;
    if (body)
      b |= 0x80;
    leb[n++] = b;
  } while (body && n < 5);
  fwrite(leb, 1, (size_t)n, ctx->text_stream);
  uint8_t locals[3] = {0x01, 0x09, wasm_mode64 ? 0x7E : 0x7F};
  fwrite(locals, 1, 3, ctx->text_stream);
  if (fwrite(wasm_buf, 1, (size_t)wasm_len, ctx->text_stream) !=
      (size_t)wasm_len) {
    fprintf(stderr, "masm: wasm: text stream full\n");
    exit(1);
  }
}

static registry_object wasm32_object = {
    REGISTRY_ENCODER, "wasm32", NULL, wasm32_encode, NULL,
};
static registry_object wasm64_object = {
    REGISTRY_ENCODER, "wasm64", NULL, wasm64_encode, NULL,
};

static void wasm_register(void) __attribute__((constructor));
static void wasm_register(void) {
  register_object(&wasm32_object);
  register_object(&wasm64_object);
}

#endif
