#ifndef MASM_RISCV_H
#define MASM_RISCV_H

#include "../core.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RV_ZERO 0
#define RV_RA 1
#define RV_SP 2
#define RV_T0 5
#define RV_A0 10

#define RV_ANY 0xFFu
#define RV_TGT 0xFEu

#define RV_WOK 1u
#define RV_NEG 2u

enum {
  RV_FIXED = 1,
  RV_ECALL,
  RV_LI,
  RV_ALU,
  RV_ALUI,
  RV_ALURI,
  RV_SHI,
  RV_DP1,
  RV_UN,
  RV_CMP,
  RV_TEST,
  RV_J,
  RV_BCC,
  RV_SETCC,
  RV_ADDR,
  RV_MEM,
  RV_PUSH,
  RV_POP,
  RV_ERR
};

typedef struct {
  const char *mn;
  uint8_t oa, ob;
  uint8_t kind;
  uint8_t ext;
  uint32_t base;
} rv_form;

#define F(mn, oa, ob, k, e, b)                                                 \
  { mn, (uint8_t)(oa), (uint8_t)(ob), (uint8_t)(k), (uint8_t)(e), b }

static const rv_form rv_tab[] = {
    F("nop", OPERAND_NULL, OPERAND_NULL, RV_FIXED, 0, 0x00000013u),
    F("ret", OPERAND_NULL, OPERAND_NULL, RV_FIXED, 0, 0x00008067u),
    F("syscall", OPERAND_NULL, OPERAND_NULL, RV_ECALL, 0, 0x00000073u),
    F("int", OPERAND_IMM, OPERAND_NULL, RV_ECALL, 0, 0x00000073u),

    F("push", OPERAND_REG, OPERAND_NULL, RV_PUSH, 0, 0),
    F("pop", OPERAND_REG, OPERAND_NULL, RV_POP, 0, 0),
    F("inc", OPERAND_REG, OPERAND_NULL, RV_DP1, 0, 0x00100013u),
    F("dec", OPERAND_REG, OPERAND_NULL, RV_DP1, 0, 0xFFF00013u),
    F("neg", OPERAND_REG, OPERAND_NULL, RV_UN, RV_WOK, 0x40000033u),
    F("not", OPERAND_REG, OPERAND_NULL, RV_UN, 0, 0xFFF04013u),

    F("mov", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x00000033u),
    F("mov", OPERAND_REG, OPERAND_IMM, RV_LI, 0, 0),
    F("add", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x00000033u),
    F("add", OPERAND_REG, OPERAND_IMM, RV_ALUI, 0, 0x00000013u),
    F("sub", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x40000033u),
    F("sub", OPERAND_REG, OPERAND_IMM, RV_ALUI, RV_WOK | RV_NEG, 0x00000013u),
    F("and", OPERAND_REG, OPERAND_REG, RV_ALU, 0, 0x02007033u),
    F("and", OPERAND_REG, OPERAND_IMM, RV_ALUI, 0, 0x00007013u),
    F("or", OPERAND_REG, OPERAND_REG, RV_ALU, 0, 0x02006033u),
    F("or", OPERAND_REG, OPERAND_IMM, RV_ALUI, 0, 0x00006013u),
    F("xor", OPERAND_REG, OPERAND_REG, RV_ALU, 0, 0x02004033u),
    F("xor", OPERAND_REG, OPERAND_IMM, RV_ALUI, 0, 0x00004013u),
    F("mul", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x02000033u),
    F("mul", OPERAND_REG, OPERAND_IMM, RV_ALURI, RV_WOK, 0x02000033u),
    F("imul", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x02000033u),
    F("imul", OPERAND_REG, OPERAND_IMM, RV_ALURI, RV_WOK, 0x02000033u),
    F("div", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x02004033u),
    F("idiv", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x02004033u),
    F("shl", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x00001033u),
    F("shl", OPERAND_REG, OPERAND_IMM, RV_SHI, RV_WOK, 0x00001013u),
    F("shr", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x00005033u),
    F("shr", OPERAND_REG, OPERAND_IMM, RV_SHI, RV_WOK, 0x00005013u),
    F("sar", OPERAND_REG, OPERAND_REG, RV_ALU, RV_WOK, 0x40005033u),
    F("sar", OPERAND_REG, OPERAND_IMM, RV_SHI, RV_WOK, 0x40005013u),
    F("xchg", OPERAND_REG, OPERAND_REG, RV_ERR, 0, 0),
    F("leave", OPERAND_NULL, OPERAND_NULL, RV_ERR, 0, 0),

    F("cmp", OPERAND_REG, OPERAND_REG, RV_CMP, 0, 0),
    F("cmp", OPERAND_REG, OPERAND_IMM, RV_CMP, 1, 0),
    F("test", OPERAND_REG, OPERAND_REG, RV_TEST, 0, 0),

    F("jmp", RV_TGT, OPERAND_NULL, RV_J, 0, 0x0000006Fu),
    F("b", RV_TGT, OPERAND_NULL, RV_J, 0, 0x0000006Fu),
    F("call", RV_TGT, OPERAND_NULL, RV_J, 0, 0x000000EFu),
    F("bl", RV_TGT, OPERAND_NULL, RV_J, 0, 0x000000EFu),

    F("je", RV_TGT, OPERAND_NULL, RV_BCC, 0, 0x00000063u),
    F("jz", RV_TGT, OPERAND_NULL, RV_BCC, 0, 0x00000063u),
    F("jne", RV_TGT, OPERAND_NULL, RV_BCC, 1, 0x00000063u),
    F("jnz", RV_TGT, OPERAND_NULL, RV_BCC, 1, 0x00000063u),
    F("jl", RV_TGT, OPERAND_NULL, RV_BCC, 4, 0x00000063u),
    F("jnge", RV_TGT, OPERAND_NULL, RV_BCC, 4, 0x00000063u),
    F("jge", RV_TGT, OPERAND_NULL, RV_BCC, 5, 0x00000063u),
    F("jnl", RV_TGT, OPERAND_NULL, RV_BCC, 5, 0x00000063u),
    F("jg", RV_TGT, OPERAND_NULL, RV_BCC, 4 | 8, 0x00000063u),
    F("jnle", RV_TGT, OPERAND_NULL, RV_BCC, 4 | 8, 0x00000063u),
    F("jle", RV_TGT, OPERAND_NULL, RV_BCC, 5 | 8, 0x00000063u),
    F("jng", RV_TGT, OPERAND_NULL, RV_BCC, 5 | 8, 0x00000063u),
    F("jb", RV_TGT, OPERAND_NULL, RV_BCC, 6, 0x00000063u),
    F("jc", RV_TGT, OPERAND_NULL, RV_BCC, 6, 0x00000063u),
    F("jnae", RV_TGT, OPERAND_NULL, RV_BCC, 6, 0x00000063u),
    F("jae", RV_TGT, OPERAND_NULL, RV_BCC, 7, 0x00000063u),
    F("jnb", RV_TGT, OPERAND_NULL, RV_BCC, 7, 0x00000063u),
    F("jnc", RV_TGT, OPERAND_NULL, RV_BCC, 7, 0x00000063u),
    F("ja", RV_TGT, OPERAND_NULL, RV_BCC, 6 | 8, 0x00000063u),
    F("jnbe", RV_TGT, OPERAND_NULL, RV_BCC, 6 | 8, 0x00000063u),
    F("jbe", RV_TGT, OPERAND_NULL, RV_BCC, 7 | 8, 0x00000063u),
    F("jna", RV_TGT, OPERAND_NULL, RV_BCC, 7 | 8, 0x00000063u),

    F("sete", OPERAND_REG, OPERAND_NULL, RV_SETCC, 0, 0),
    F("setz", OPERAND_REG, OPERAND_NULL, RV_SETCC, 0, 0),
    F("setne", OPERAND_REG, OPERAND_NULL, RV_SETCC, 1, 0),
    F("setnz", OPERAND_REG, OPERAND_NULL, RV_SETCC, 1, 0),
    F("setl", OPERAND_REG, OPERAND_NULL, RV_SETCC, 4, 0),
    F("setnge", OPERAND_REG, OPERAND_NULL, RV_SETCC, 4, 0),
    F("setge", OPERAND_REG, OPERAND_NULL, RV_SETCC, 5, 0),
    F("setnl", OPERAND_REG, OPERAND_NULL, RV_SETCC, 5, 0),
    F("setg", OPERAND_REG, OPERAND_NULL, RV_SETCC, 12, 0),
    F("setnle", OPERAND_REG, OPERAND_NULL, RV_SETCC, 12, 0),
    F("setle", OPERAND_REG, OPERAND_NULL, RV_SETCC, 14, 0),
    F("setng", OPERAND_REG, OPERAND_NULL, RV_SETCC, 14, 0),
    F("setb", OPERAND_REG, OPERAND_NULL, RV_SETCC, 2, 0),
    F("setc", OPERAND_REG, OPERAND_NULL, RV_SETCC, 2, 0),
    F("setnae", OPERAND_REG, OPERAND_NULL, RV_SETCC, 2, 0),
    F("setae", OPERAND_REG, OPERAND_NULL, RV_SETCC, 3, 0),
    F("setnb", OPERAND_REG, OPERAND_NULL, RV_SETCC, 3, 0),
    F("setnc", OPERAND_REG, OPERAND_NULL, RV_SETCC, 3, 0),
    F("seta", OPERAND_REG, OPERAND_NULL, RV_SETCC, 7, 0),
    F("setnbe", OPERAND_REG, OPERAND_NULL, RV_SETCC, 7, 0),
    F("setbe", OPERAND_REG, OPERAND_NULL, RV_SETCC, 6, 0),
    F("setna", OPERAND_REG, OPERAND_NULL, RV_SETCC, 6, 0),

    F("ldr", OPERAND_REG, OPERAND_REL, RV_ADDR, 0, 0x00000017u),
    F("lea", OPERAND_REG, OPERAND_REL, RV_ADDR, 0, 0x00000017u),
    F("load", OPERAND_REG, OPERAND_REL, RV_MEM, 1, 0),
    F("str", OPERAND_REG, OPERAND_REL, RV_MEM, 0, 0),
};

#undef F

static int rv_cmp_a, rv_cmp_b, rv_cmp_kind;
static int64_t rv_cmp_imm;

static void rv_emit(context *ctx, uint32_t w) {
  uint8_t b[4] = {(uint8_t)w, (uint8_t)(w >> 8), (uint8_t)(w >> 16),
                  (uint8_t)(w >> 24)};
  if (ctx->text_stream && fwrite(b, 1, 4, ctx->text_stream) != 4) {
    fprintf(stderr, "masm: riscv: text stream full\n");
    exit(1);
  }
  *ctx->current_text_adrs += 4;
}

static void rv_rel(context *ctx, uint64_t at, SECTION sec, const char *name,
                   int64_t addend, uint8_t type) {
  if (ctx->text_stream) {
    rel *rr = ctx_new_rel(ctx);
    rr->adrs = at;
    rr->section = sec;
    rr->name = name;
    rr->addend = addend;
    rr->type = type;
  }
}

static uint8_t rv_reg(uint64_t idx) {
  static const uint8_t rv_regs[11] = {10, 11, 12, 13, 14, 15, 16, 17, 5, 2, 8};
  return rv_regs[idx & 15];
}

static int rv_match(uint8_t want, OPERAND got) {
  if (want == RV_ANY)
    return 1;
  if (want == RV_TGT)
    return got == OPERAND_REL || got == OPERAND_IMM;
  return want == (uint8_t)got;
}

static const rv_form *rv_find(const char *mn, OPERAND oa, OPERAND ob) {
  for (size_t i = 0; i < sizeof rv_tab / sizeof rv_tab[0]; i++) {
    const rv_form *f = &rv_tab[i];
    if (strcmp(f->mn, mn) != 0)
      continue;
    if (rv_match(f->oa, oa) && rv_match(f->ob, ob))
      return f;
  }
  return NULL;
}

static void rv_die(const char *mn, const char *why) {
  fprintf(stderr, "masm: riscv: '%s' not supported (%s)\n", mn, why);
  exit(1);
}

static uint32_t rv_i(int32_t imm, uint8_t rs1, uint8_t f3, uint8_t rd,
                     uint32_t op) {
  return (((uint32_t)imm & 0xFFFu) << 20) | ((uint32_t)rs1 << 15) |
         ((uint32_t)f3 << 12) | ((uint32_t)rd << 7) | op;
}

static uint32_t rv_r(uint8_t rs2, uint8_t rs1, uint8_t f3, uint8_t rd,
                     uint32_t op, uint32_t f7) {
  return (f7 << 25) | ((uint32_t)rs2 << 20) | ((uint32_t)rs1 << 15) |
         ((uint32_t)f3 << 12) | ((uint32_t)rd << 7) | op;
}

static uint32_t rv_s(int32_t imm, uint8_t rs2, uint8_t rs1, uint8_t f3,
                     uint32_t op) {
  uint32_t i = (uint32_t)imm;
  return (((i >> 5) & 0x7Fu) << 25) | ((uint32_t)rs2 << 20) |
         ((uint32_t)rs1 << 15) | ((uint32_t)f3 << 12) | ((i & 0x1Fu) << 7) |
         op;
}

static uint32_t rv_b(int32_t imm, uint8_t rs2, uint8_t rs1, uint8_t f3) {
  uint32_t i = (uint32_t)imm;
  return (((i >> 12) & 1u) << 31) | (((i >> 5) & 0x3Fu) << 25) |
         ((uint32_t)rs2 << 20) | ((uint32_t)rs1 << 15) | ((uint32_t)f3 << 12) |
         (((i >> 1) & 0xFu) << 8) | (((i >> 11) & 1u) << 7) | 0x63u;
}

static uint32_t rv_j(int32_t imm, uint8_t rd) {
  uint32_t i = (uint32_t)imm;
  return (((i >> 20) & 1u) << 31) | (((i >> 1) & 0x3FFu) << 21) |
         (((i >> 11) & 1u) << 20) | (((i >> 12) & 0xFFu) << 12) |
         ((uint32_t)rd << 7) | 0x6Fu;
}

static int64_t rv_sch(uint64_t v, int pos, int bits) {
  int64_t c = (int64_t)((v >> pos) & ((1ull << bits) - 1ull));
  if (c >= (1ll << (bits - 1)))
    c -= (1ll << bits);
  return c;
}

static void rv_li(context *ctx, int rv64, uint8_t rd, uint64_t v64) {
  int64_t v = (int64_t)v64;
  if (v >= -2048 && v <= 2047) {
    rv_emit(ctx, rv_i((int32_t)v, RV_ZERO, 0, rd, 0x13u));
    return;
  }
  int32_t lo32 = (int32_t)(uint32_t)v64;
  if (!rv64 || (int64_t)lo32 == v) {
    int32_t hi = (lo32 + 0x800) >> 12;
    int32_t lo = lo32 - (hi << 12);
    rv_emit(ctx, (((uint32_t)hi & 0xFFFFFu) << 12) | ((uint32_t)rd << 7) |
                     0x37u);
    rv_emit(ctx, rv_i(lo, rd, 0, rd, 0x13u));
    return;
  }
  static const int sh[5] = {12, 12, 12, 12, 8};
  static const int po[5] = {44, 32, 20, 8, 0};
  static const int bi[5] = {12, 12, 12, 12, 8};
  rv_emit(ctx, rv_i((int32_t)rv_sch(v64, 56, 8), RV_ZERO, 0, rd, 0x13u));
  for (int i = 0; i < 5; i++) {
    rv_emit(ctx, rv_i(sh[i], rd, 1, rd, 0x13u));
    rv_emit(ctx, rv_i((int32_t)rv_sch(v64, po[i], bi[i]), rd, 0, rd, 0x13u));
  }
}

static void rv_encode_mode(context *ctx, char *mnemonic, WIDTH width,
                           OPERAND oa, uint64_t ia, rel ra, OPERAND ob,
                           uint64_t ib, rel rb, int rv64) {
  if (mnemonic[0] == '.')
    return;
  const rv_form *f = rv_find(mnemonic, oa, ob);
  if (!f) {
    masm_dief("riscv", "no form for '%s'", mnemonic);
  }
  uint8_t rd = rv_reg(ia & 15);
  uint8_t rs = rv_reg(ib & 15);

  if (width == WIDTH_WORD || width == WIDTH_BYTE)
    if (f->kind != RV_MEM)
      rv_die(mnemonic, "word/byte width unsupported");
  if (!rv64 && width == WIDTH_QWORD)
    rv_die(mnemonic, "qword needs rv64gc");
  int is32 = (width == WIDTH_DWORD);
  int w = rv64 && is32;

  switch (f->kind) {
  case RV_FIXED:
  case RV_ECALL:
    rv_emit(ctx, f->base);
    break;
  case RV_LI:
    rv_li(ctx, rv64, rd, ib);
    break;
  case RV_ALU: {
    uint32_t base = (f->ext & RV_WOK) && w ? f->base | 0x08u : f->base;
    rv_emit(ctx, base | ((uint32_t)rs << 20) | ((uint32_t)rd << 15) |
                       ((uint32_t)rd << 7));
    break;
  }
  case RV_ALURI: {
    rv_li(ctx, rv64, RV_T0, ib);
    uint32_t base = (f->ext & RV_WOK) && w ? f->base | 0x08u : f->base;
    rv_emit(ctx, base | ((uint32_t)RV_T0 << 20) | ((uint32_t)rd << 15) |
                       ((uint32_t)rd << 7));
    break;
  }
  case RV_ALUI: {
    int64_t v = (int64_t)ib;
    uint32_t f3 = (f->base >> 12) & 7u;
    if (v >= -2048 && v <= 2047) {
      if (f->ext & RV_NEG)
        v = -v;
      uint32_t base = (f->ext & RV_WOK) && w ? f->base | 0x08u : f->base;
      rv_emit(ctx, rv_i((int32_t)v, rd, f3, rd, base & 0x7Fu));
    } else {
      rv_li(ctx, rv64, RV_T0, ib);
      uint32_t f7 = 0;
      if (f3 == 4 || f3 == 6)
        f7 = 1;
      if ((f->ext & RV_NEG) && f3 == 0)
        f7 = 0x20;
      uint32_t op = w ? 0x3Bu : 0x33u;
      rv_emit(ctx, rv_r(RV_T0, rd, f3, rd, op, f7));
    }
    break;
  }
  case RV_SHI: {
    if ((int64_t)ib < 0 || (int64_t)ib > (rv64 ? 63 : 31)) {
      masm_dief("riscv", "shift count out of range");
    }
    uint32_t shamt = (uint32_t)ib & (rv64 ? 63u : 31u);
    uint32_t base = (f->ext & RV_WOK) && w ? f->base | 0x08u : f->base;
    rv_emit(ctx, base | (shamt << 20) | ((uint32_t)rd << 15) |
                       ((uint32_t)rd << 7));
    break;
  }
  case RV_DP1:
  case RV_UN: {
    uint32_t base = (f->ext & RV_WOK) && w ? f->base | 0x08u : f->base;
    rv_emit(ctx, base | ((uint32_t)rd << 15) | ((uint32_t)rd << 7));
    break;
  }
  case RV_CMP:
    rv_cmp_a = (int)ia;
    rv_cmp_b = (int)ib;
    rv_cmp_kind = f->ext ? 3 : 1;
    rv_cmp_imm = (int64_t)ib;
    break;
  case RV_TEST:
    rv_cmp_a = (int)ia;
    rv_cmp_b = (int)ib;
    rv_cmp_kind = 2;
    break;
  case RV_J: {
    if (oa == OPERAND_REL && ra.section == SECTION_TEXT) {
      int64_t off = (int64_t)ra.adrs - (int64_t)*ctx->current_text_adrs;
      if (off < -(1 << 20) || off >= (1 << 20))
        rv_die(mnemonic, "jal out of range");
      rv_emit(ctx, rv_j((int32_t)off, (uint8_t)((f->base >> 7) & 1u)));
    } else if (oa == OPERAND_REL) {
      rv_rel(ctx, *ctx->current_text_adrs, ra.section, ra.name,
             (int64_t)ra.adrs, RV_JAL);
      rv_emit(ctx, f->base);
    } else {
      rv_die(mnemonic, "label operand required");
    }
    break;
  }
  case RV_BCC: {
    if (rv_cmp_kind == 0)
      rv_die(mnemonic, "conditional branch needs a preceding cmp");
    uint8_t f3 = f->ext & 7u;
    int swap = f->ext & 8u;
    uint8_t a = rv_reg(rv_cmp_a);
    uint8_t b = rv_reg(rv_cmp_b);
    if (rv_cmp_kind == 3) {
      rv_li(ctx, rv64, RV_T0, (uint64_t)rv_cmp_imm);
      b = RV_T0;
    } else if (rv_cmp_kind == 2) {
      if (f3 != 0 && f3 != 1)
        rv_die(mnemonic, "test only supports je/jne");
      rv_emit(ctx, rv_r(b, a, 7, RV_T0, 0x33u, 0u));
      a = RV_T0;
      b = RV_ZERO;
      f3 = f3 == 0 ? 0u : 1u;
    } else if (swap) {
      uint8_t t = a;
      a = b;
      b = t;
    }
    if (oa == OPERAND_REL && ra.section == SECTION_TEXT) {
      int64_t off = (int64_t)ra.adrs - (int64_t)*ctx->current_text_adrs;
      if (off < -(1 << 12) || off >= (1 << 12))
        rv_die(mnemonic, "branch out of range");
      rv_emit(ctx, rv_b((int32_t)off, b, a, f3));
    } else if (oa == OPERAND_REL) {
      rv_rel(ctx, *ctx->current_text_adrs, ra.section, ra.name,
             (int64_t)ra.adrs, RV_BRANCH);
      rv_emit(ctx, rv_b(0, b, a, f3));
    } else {
      rv_die(mnemonic, "label operand required");
    }
    break;
  }
  case RV_SETCC: {
    if (rv_cmp_kind == 0)
      rv_die(mnemonic, "setcc needs a preceding cmp");
    uint8_t a = rv_reg(rv_cmp_a);
    uint8_t b = rv_reg(rv_cmp_b);
    if (rv_cmp_kind == 3) {
      rv_li(ctx, rv64, RV_T0, (uint64_t)rv_cmp_imm);
      b = RV_T0;
    }
    uint8_t f3 = 2;
    int flip = 0;
    int swap = 0;
    int xor_first = 0;
    switch (f->ext) {
    case 0:
      xor_first = 1;
      break;
    case 1:
      xor_first = 1;
      flip = 1;
      break;
    case 4:
      break;
    case 5:
      flip = 1;
      break;
    case 12:
      swap = 1;
      break;
    case 14:
      swap = 1;
      flip = 1;
      break;
    case 2:
      f3 = 3;
      break;
    case 3:
      f3 = 3;
      flip = 1;
      break;
    case 7:
      f3 = 3;
      swap = 1;
      break;
    case 6:
      f3 = 3;
      swap = 1;
      flip = 1;
      break;
    default:
      rv_die(mnemonic, "condition");
    }
    if (xor_first) {
      rv_emit(ctx, rv_r(b, a, 4, rd, 0x33u, 0u));
      if (f->ext == 0)
        rv_emit(ctx, rv_i(1, rd, 3, rd, 0x13u));
      else
        rv_emit(ctx, rv_r(rd, RV_ZERO, 3, rd, 0x33u, 0u));
      break;
    }
    if (swap) {
      uint8_t t = a;
      a = b;
      b = t;
    }
    rv_emit(ctx, rv_r(b, a, f3, rd, 0x33u, 0u));
    if (flip)
      rv_emit(ctx, rv_i(1, rd, 4, rd, 0x13u));
    break;
  }
  case RV_ADDR: {
    if (rb.section == SECTION_TEXT) {
      int64_t off = (int64_t)rb.adrs - (int64_t)*ctx->current_text_adrs;
      int32_t hi = (int32_t)((off + 0x800) >> 12);
      int32_t lo = (int32_t)off - (hi << 12);
      rv_emit(ctx, (((uint32_t)hi & 0xFFFFFu) << 12) | ((uint32_t)rd << 7) |
                       0x17u);
      rv_emit(ctx, rv_i(lo, rd, 0, rd, 0x13u));
    } else {
      rv_rel(ctx, *ctx->current_text_adrs, rb.section, rb.name,
             (int64_t)rb.adrs, RV_HI20);
      rv_emit(ctx, ((uint32_t)rd << 7) | 0x37u);
      rv_rel(ctx, *ctx->current_text_adrs, rb.section, rb.name,
             (int64_t)rb.adrs, RV_LO12);
      rv_emit(ctx, rv_i(0, rd, 0, rd, 0x13u));
    }
    break;
  }
  case RV_MEM: {
    int is_load = f->ext;
    uint8_t f3;
    if (width == WIDTH_BYTE)
      f3 = 0;
    else if (width == WIDTH_WORD)
      f3 = 1;
    else if (width == WIDTH_DWORD)
      f3 = 2;
    else
      f3 = rv64 ? 3 : 2;
    uint8_t ab = is_load ? rd : RV_T0;
    if (rb.section == SECTION_TEXT) {
      int64_t off = (int64_t)rb.adrs - (int64_t)*ctx->current_text_adrs;
      int32_t hi = (int32_t)((off + 0x800) >> 12);
      int32_t lo = (int32_t)off - (hi << 12);
      rv_emit(ctx, (((uint32_t)hi & 0xFFFFFu) << 12) | ((uint32_t)ab << 7) |
                       0x17u);
      rv_emit(ctx, rv_i(lo, ab, 0, ab, 0x13u));
    } else {
      rv_rel(ctx, *ctx->current_text_adrs, rb.section, rb.name,
             (int64_t)rb.adrs, RV_HI20);
      rv_emit(ctx, ((uint32_t)ab << 7) | 0x37u);
      rv_rel(ctx, *ctx->current_text_adrs, rb.section, rb.name,
             (int64_t)rb.adrs, RV_LO12);
      rv_emit(ctx, rv_i(0, ab, 0, ab, 0x13u));
    }
    if (is_load)
      rv_emit(ctx, rv_i(0, ab, f3, rd, 0x03u));
    else
      rv_emit(ctx, rv_s(0, rd, ab, f3, 0x23u));
    break;
  }
  case RV_PUSH:
    rv_emit(ctx, rv_i(-16, RV_SP, 0, RV_SP, 0x13u));
    rv_emit(ctx, rv_s(0, rd, RV_SP, rv64 ? 3u : 2u, 0x23u));
    break;
  case RV_POP:
    rv_emit(ctx, rv_i(0, RV_SP, rv64 ? 3u : 2u, rd, 0x03u));
    rv_emit(ctx, rv_i(16, RV_SP, 0, RV_SP, 0x13u));
    break;
  default:
    rv_die(mnemonic, "encode failed");
  }
}

static void rv64_encode(context *ctx, char *mnemonic, WIDTH width, OPERAND oa,
                        uint64_t ia, rel ra, OPERAND ob, uint64_t ib, rel rb) {
  rv_encode_mode(ctx, mnemonic, width, oa, ia, ra, ob, ib, rb, 1);
}

static void rv32_encode(context *ctx, char *mnemonic, WIDTH width, OPERAND oa,
                        uint64_t ia, rel ra, OPERAND ob, uint64_t ib, rel rb) {
  rv_encode_mode(ctx, mnemonic, width, oa, ia, ra, ob, ib, rb, 0);
}

static registry_object rv64_object = {
    REGISTRY_ENCODER, "rv64gc", NULL, rv64_encode, NULL,
};
static registry_object rv32_object = {
    REGISTRY_ENCODER, "rv32gc", NULL, rv32_encode, NULL,
};

static void riscv_register(void) __attribute__((constructor));
static void riscv_register(void) {
  register_object(&rv64_object);
  register_object(&rv32_object);
}

#endif
