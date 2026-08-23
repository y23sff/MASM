#ifndef MASM_ARM64_H
#define MASM_ARM64_H

#include "../core.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define A64_ZR 31
#define A64_IP 17
#define A64_SP 31

#define A64_ANY 0xFFu
#define A64_TGT 0xFEu

enum {
  A64_FIXED = 1,
  A64_SVC,
  A64_MOV_I,
  A64_DP3,
  A64_MOV,
  A64_UN,
  A64_CC,
  A64_DP_RI,
  A64_CC_RI,
  A64_DP1,
  A64_MUL_RR,
  A64_MUL_RI,
  A64_DIV,
  A64_SH_RR,
  A64_ROL_RR,
  A64_SH_I,
  A64_ROL_I,
  A64_PUSH,
  A64_POP,
  A64_BR,
  A64_BCC,
  A64_SETCC,
  A64_CMOV,
  A64_ADR,
  A64_MEM,
  A64_SXTW,
  A64_ERR
};

typedef struct {
  const char *mn;
  uint8_t oa, ob;
  uint8_t kind;
  uint8_t ext;
  uint64_t base;
} a64_form;

#define F(mn, oa, ob, k, e, b)                                                 \
  { mn, (uint8_t)(oa), (uint8_t)(ob), (uint8_t)(k), (uint8_t)(e), b }

static const a64_form a64_tab[] = {
    F("nop", OPERAND_NULL, OPERAND_NULL, A64_FIXED, 0, 0xD503201Full),
    F("ret", OPERAND_NULL, OPERAND_NULL, A64_FIXED, 0, 0xD65F03C0ull),
    F("syscall", OPERAND_NULL, OPERAND_NULL, A64_SVC, 0, 0xD4000001ull),
    F("int", OPERAND_IMM, OPERAND_NULL, A64_SVC, 0, 0xD4000001ull),

    F("push", OPERAND_REG, OPERAND_NULL, A64_PUSH, 0, 0xF81F0FE0ull),
    F("pop", OPERAND_REG, OPERAND_NULL, A64_POP, 0, 0xF85047E0ull),
    F("inc", OPERAND_REG, OPERAND_NULL, A64_DP1, 0, 0x91000400ull),
    F("dec", OPERAND_REG, OPERAND_NULL, A64_DP1, 0, 0xD1000400ull),
    F("neg", OPERAND_REG, OPERAND_NULL, A64_UN, 0, 0xCB0003E0ull),
    F("not", OPERAND_REG, OPERAND_NULL, A64_UN, 0, 0xAA2003E0ull),

    F("mov", OPERAND_REG, OPERAND_REG, A64_MOV, 0, 0xAA0003E0ull),
    F("movzx", OPERAND_REG, OPERAND_REG, A64_MOV, 0, 0xAA0003E0ull),
    F("movsx", OPERAND_REG, OPERAND_REG, A64_MOV, 0, 0xAA0003E0ull),
    F("mov", OPERAND_REG, OPERAND_IMM, A64_MOV_I, 0, 0),
    F("movsxd", OPERAND_REG, OPERAND_REG, A64_SXTW, 0, 0x93407C00ull),
    F("cdqe", OPERAND_REG, OPERAND_REG, A64_SXTW, 0, 0x93407C00ull),

    F("add", OPERAND_REG, OPERAND_REG, A64_DP3, 0, 0x8B000000ull),
    F("add", OPERAND_REG, OPERAND_IMM, A64_DP_RI, 0, 0x91000000ull),
    F("sub", OPERAND_REG, OPERAND_REG, A64_DP3, 0, 0xCB000000ull),
    F("sub", OPERAND_REG, OPERAND_IMM, A64_DP_RI, 0, 0xD1000000ull),
    F("and", OPERAND_REG, OPERAND_REG, A64_DP3, 0, 0x8A000000ull),
    F("or", OPERAND_REG, OPERAND_REG, A64_DP3, 0, 0xAA000000ull),
    F("xor", OPERAND_REG, OPERAND_REG, A64_DP3, 0, 0xCA000000ull),
    F("cmp", OPERAND_REG, OPERAND_REG, A64_CC, 0, 0xEB00001Full),
    F("cmp", OPERAND_REG, OPERAND_IMM, A64_CC_RI, 0, 0xF100001Full),
    F("test", OPERAND_REG, OPERAND_REG, A64_CC, 0, 0x6A00001Full),

    F("mul", OPERAND_REG, OPERAND_REG, A64_MUL_RR, 0, 0x9B007C00ull),
    F("mul", OPERAND_REG, OPERAND_IMM, A64_MUL_RI, 0, 0),
    F("imul", OPERAND_REG, OPERAND_REG, A64_MUL_RR, 0, 0x9B007C00ull),
    F("imul", OPERAND_REG, OPERAND_IMM, A64_MUL_RI, 0, 0),
    F("div", OPERAND_REG, OPERAND_REG, A64_DIV, 0, 0x9AC00C00ull),
    F("idiv", OPERAND_REG, OPERAND_REG, A64_DIV, 0, 0x9AC00C00ull),
    F("xchg", OPERAND_REG, OPERAND_REG, A64_ERR, 0, 0),

    F("shl", OPERAND_REG, OPERAND_REG, A64_SH_RR, 0, 0x9AC02000ull),
    F("sal", OPERAND_REG, OPERAND_REG, A64_SH_RR, 0, 0x9AC02000ull),
    F("shl", OPERAND_REG, OPERAND_IMM, A64_SH_I, 0, 0),
    F("sal", OPERAND_REG, OPERAND_IMM, A64_SH_I, 0, 0),
    F("shr", OPERAND_REG, OPERAND_REG, A64_SH_RR, 1, 0x9AC02400ull),
    F("shr", OPERAND_REG, OPERAND_IMM, A64_SH_I, 1, 0),
    F("sar", OPERAND_REG, OPERAND_REG, A64_SH_RR, 2, 0x9AC02800ull),
    F("sar", OPERAND_REG, OPERAND_IMM, A64_SH_I, 2, 0),
    F("ror", OPERAND_REG, OPERAND_REG, A64_SH_RR, 3, 0x9AC02C00ull),
    F("ror", OPERAND_REG, OPERAND_IMM, A64_SH_I, 3, 0x93C00000ull),
    F("rol", OPERAND_REG, OPERAND_REG, A64_ROL_RR, 0, 0),
    F("rol", OPERAND_REG, OPERAND_IMM, A64_ROL_I, 0, 0x93C00000ull),

    F("jmp", A64_TGT, OPERAND_NULL, A64_BR, 0, 0x14000000ull),
    F("b", A64_TGT, OPERAND_NULL, A64_BR, 0, 0x14000000ull),
    F("call", A64_TGT, OPERAND_NULL, A64_BR, 0, 0x94000000ull),
    F("bl", A64_TGT, OPERAND_NULL, A64_BR, 0, 0x94000000ull),

    F("jo", A64_TGT, OPERAND_NULL, A64_BCC, 6, 0x54000000ull),
    F("jno", A64_TGT, OPERAND_NULL, A64_BCC, 7, 0x54000000ull),
    F("je", A64_TGT, OPERAND_NULL, A64_BCC, 0, 0x54000000ull),
    F("jz", A64_TGT, OPERAND_NULL, A64_BCC, 0, 0x54000000ull),
    F("jne", A64_TGT, OPERAND_NULL, A64_BCC, 1, 0x54000000ull),
    F("jnz", A64_TGT, OPERAND_NULL, A64_BCC, 1, 0x54000000ull),
    F("jb", A64_TGT, OPERAND_NULL, A64_BCC, 3, 0x54000000ull),
    F("jc", A64_TGT, OPERAND_NULL, A64_BCC, 3, 0x54000000ull),
    F("jnae", A64_TGT, OPERAND_NULL, A64_BCC, 3, 0x54000000ull),
    F("jae", A64_TGT, OPERAND_NULL, A64_BCC, 2, 0x54000000ull),
    F("jnb", A64_TGT, OPERAND_NULL, A64_BCC, 2, 0x54000000ull),
    F("jnc", A64_TGT, OPERAND_NULL, A64_BCC, 2, 0x54000000ull),
    F("jbe", A64_TGT, OPERAND_NULL, A64_BCC, 9, 0x54000000ull),
    F("jna", A64_TGT, OPERAND_NULL, A64_BCC, 9, 0x54000000ull),
    F("ja", A64_TGT, OPERAND_NULL, A64_BCC, 8, 0x54000000ull),
    F("jnbe", A64_TGT, OPERAND_NULL, A64_BCC, 8, 0x54000000ull),
    F("js", A64_TGT, OPERAND_NULL, A64_BCC, 4, 0x54000000ull),
    F("jns", A64_TGT, OPERAND_NULL, A64_BCC, 5, 0x54000000ull),
    F("jl", A64_TGT, OPERAND_NULL, A64_BCC, 11, 0x54000000ull),
    F("jnge", A64_TGT, OPERAND_NULL, A64_BCC, 11, 0x54000000ull),
    F("jge", A64_TGT, OPERAND_NULL, A64_BCC, 10, 0x54000000ull),
    F("jnl", A64_TGT, OPERAND_NULL, A64_BCC, 10, 0x54000000ull),
    F("jle", A64_TGT, OPERAND_NULL, A64_BCC, 13, 0x54000000ull),
    F("jng", A64_TGT, OPERAND_NULL, A64_BCC, 13, 0x54000000ull),
    F("jg", A64_TGT, OPERAND_NULL, A64_BCC, 12, 0x54000000ull),
    F("jnle", A64_TGT, OPERAND_NULL, A64_BCC, 12, 0x54000000ull),

    F("seto", OPERAND_REG, OPERAND_NULL, A64_SETCC, 6, 0x9A9F07E0ull),
    F("setno", OPERAND_REG, OPERAND_NULL, A64_SETCC, 7, 0x9A9F07E0ull),
    F("sete", OPERAND_REG, OPERAND_NULL, A64_SETCC, 0, 0x9A9F07E0ull),
    F("setz", OPERAND_REG, OPERAND_NULL, A64_SETCC, 0, 0x9A9F07E0ull),
    F("setne", OPERAND_REG, OPERAND_NULL, A64_SETCC, 1, 0x9A9F07E0ull),
    F("setnz", OPERAND_REG, OPERAND_NULL, A64_SETCC, 1, 0x9A9F07E0ull),
    F("setb", OPERAND_REG, OPERAND_NULL, A64_SETCC, 3, 0x9A9F07E0ull),
    F("setc", OPERAND_REG, OPERAND_NULL, A64_SETCC, 3, 0x9A9F07E0ull),
    F("setnae", OPERAND_REG, OPERAND_NULL, A64_SETCC, 3, 0x9A9F07E0ull),
    F("setae", OPERAND_REG, OPERAND_NULL, A64_SETCC, 2, 0x9A9F07E0ull),
    F("setnb", OPERAND_REG, OPERAND_NULL, A64_SETCC, 2, 0x9A9F07E0ull),
    F("setnc", OPERAND_REG, OPERAND_NULL, A64_SETCC, 2, 0x9A9F07E0ull),
    F("setbe", OPERAND_REG, OPERAND_NULL, A64_SETCC, 9, 0x9A9F07E0ull),
    F("setna", OPERAND_REG, OPERAND_NULL, A64_SETCC, 9, 0x9A9F07E0ull),
    F("seta", OPERAND_REG, OPERAND_NULL, A64_SETCC, 8, 0x9A9F07E0ull),
    F("setnbe", OPERAND_REG, OPERAND_NULL, A64_SETCC, 8, 0x9A9F07E0ull),
    F("sets", OPERAND_REG, OPERAND_NULL, A64_SETCC, 4, 0x9A9F07E0ull),
    F("setns", OPERAND_REG, OPERAND_NULL, A64_SETCC, 5, 0x9A9F07E0ull),
    F("setl", OPERAND_REG, OPERAND_NULL, A64_SETCC, 11, 0x9A9F07E0ull),
    F("setnge", OPERAND_REG, OPERAND_NULL, A64_SETCC, 11, 0x9A9F07E0ull),
    F("setge", OPERAND_REG, OPERAND_NULL, A64_SETCC, 10, 0x9A9F07E0ull),
    F("setnl", OPERAND_REG, OPERAND_NULL, A64_SETCC, 10, 0x9A9F07E0ull),
    F("setle", OPERAND_REG, OPERAND_NULL, A64_SETCC, 13, 0x9A9F07E0ull),
    F("setng", OPERAND_REG, OPERAND_NULL, A64_SETCC, 13, 0x9A9F07E0ull),
    F("setg", OPERAND_REG, OPERAND_NULL, A64_SETCC, 12, 0x9A9F07E0ull),
    F("setnle", OPERAND_REG, OPERAND_NULL, A64_SETCC, 12, 0x9A9F07E0ull),

    F("cmovo", OPERAND_REG, OPERAND_REG, A64_CMOV, 6, 0x9A800400ull),
    F("cmovno", OPERAND_REG, OPERAND_REG, A64_CMOV, 7, 0x9A800400ull),
    F("cmove", OPERAND_REG, OPERAND_REG, A64_CMOV, 0, 0x9A800400ull),
    F("cmovz", OPERAND_REG, OPERAND_REG, A64_CMOV, 0, 0x9A800400ull),
    F("cmovne", OPERAND_REG, OPERAND_REG, A64_CMOV, 1, 0x9A800400ull),
    F("cmovnz", OPERAND_REG, OPERAND_REG, A64_CMOV, 1, 0x9A800400ull),
    F("cmovb", OPERAND_REG, OPERAND_REG, A64_CMOV, 3, 0x9A800400ull),
    F("cmovc", OPERAND_REG, OPERAND_REG, A64_CMOV, 3, 0x9A800400ull),
    F("cmovnae", OPERAND_REG, OPERAND_REG, A64_CMOV, 3, 0x9A800400ull),
    F("cmovae", OPERAND_REG, OPERAND_REG, A64_CMOV, 2, 0x9A800400ull),
    F("cmovnb", OPERAND_REG, OPERAND_REG, A64_CMOV, 2, 0x9A800400ull),
    F("cmovnc", OPERAND_REG, OPERAND_REG, A64_CMOV, 2, 0x9A800400ull),
    F("cmovbe", OPERAND_REG, OPERAND_REG, A64_CMOV, 9, 0x9A800400ull),
    F("cmovna", OPERAND_REG, OPERAND_REG, A64_CMOV, 9, 0x9A800400ull),
    F("cmova", OPERAND_REG, OPERAND_REG, A64_CMOV, 8, 0x9A800400ull),
    F("cmovnbe", OPERAND_REG, OPERAND_REG, A64_CMOV, 8, 0x9A800400ull),
    F("cmovs", OPERAND_REG, OPERAND_REG, A64_CMOV, 4, 0x9A800400ull),
    F("cmovns", OPERAND_REG, OPERAND_REG, A64_CMOV, 5, 0x9A800400ull),
    F("cmovl", OPERAND_REG, OPERAND_REG, A64_CMOV, 11, 0x9A800400ull),
    F("cmovnge", OPERAND_REG, OPERAND_REG, A64_CMOV, 11, 0x9A800400ull),
    F("cmovge", OPERAND_REG, OPERAND_REG, A64_CMOV, 10, 0x9A800400ull),
    F("cmovnl", OPERAND_REG, OPERAND_REG, A64_CMOV, 10, 0x9A800400ull),
    F("cmovle", OPERAND_REG, OPERAND_REG, A64_CMOV, 13, 0x9A800400ull),
    F("cmovng", OPERAND_REG, OPERAND_REG, A64_CMOV, 13, 0x9A800400ull),
    F("cmovg", OPERAND_REG, OPERAND_REG, A64_CMOV, 12, 0x9A800400ull),
    F("cmovnle", OPERAND_REG, OPERAND_REG, A64_CMOV, 12, 0x9A800400ull),

    F("ldr", OPERAND_REG, OPERAND_REL, A64_ADR, 0, 0x10000000ull),
    F("lea", OPERAND_REG, OPERAND_REL, A64_ADR, 0, 0x10000000ull),
    F("load", OPERAND_REG, OPERAND_REL, A64_MEM, 1, 0),
    F("str", OPERAND_REG, OPERAND_REL, A64_MEM, 0, 0),
};

#undef F

static void a64_emit(context *ctx, uint32_t w) {
  uint8_t b[4] = {(uint8_t)w, (uint8_t)(w >> 8), (uint8_t)(w >> 16),
                  (uint8_t)(w >> 24)};
  if (ctx->text_stream && fwrite(b, 1, 4, ctx->text_stream) != 4) {
    fprintf(stderr, "masm: arm64: text stream full\n");
    exit(1);
  }
  *ctx->current_text_adrs += 4;
}

static void a64_rel(context *ctx, uint64_t at, SECTION sec, const char *name,
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

static int a64_is32(WIDTH w) { return w == WIDTH_DWORD; }

static uint32_t a64_w(WIDTH w, uint64_t b) {
  return a64_is32(w) ? (uint32_t)(b & 0x7FFFFFFFull) : (uint32_t)b;
}

static uint32_t a64_bw(WIDTH w, uint64_t b) {
  return a64_is32(w) ? (uint32_t)(b & ~(0x80400000ull)) : (uint32_t)b;
}

static int a64_match(uint8_t want, OPERAND got) {
  if (want == A64_ANY)
    return 1;
  if (want == A64_TGT)
    return got == OPERAND_REL || got == OPERAND_IMM;
  return want == (uint8_t)got;
}

static const a64_form *a64_find(const char *mn, OPERAND oa, OPERAND ob) {
  for (size_t i = 0; i < sizeof a64_tab / sizeof a64_tab[0]; i++) {
    const a64_form *f = &a64_tab[i];
    if (strcmp(f->mn, mn) != 0)
      continue;
    if (a64_match(f->oa, oa) && a64_match(f->ob, ob))
      return f;
  }
  return NULL;
}

static void a64_mov_imm(context *ctx, WIDTH w, uint8_t rd, uint64_t val) {
  int bits = a64_is32(w) ? 32 : 64;
  uint64_t v = a64_is32(w) ? (val & 0xFFFFFFFFull) : val;
  uint32_t movz = a64_is32(w) ? 0x52800000u : 0xD2800000u;
  uint32_t movk = a64_is32(w) ? 0x72800000u : 0xF2800000u;
  int first = 1;
  for (int hw = 0; hw < bits / 16; hw++) {
    uint32_t part = (uint32_t)((v >> (16 * hw)) & 0xFFFFu);
    if (first) {
      a64_emit(ctx, movz | ((uint32_t)hw << 21) | (part << 5) | rd);
      first = 0;
    } else if (part) {
      a64_emit(ctx, movk | ((uint32_t)hw << 21) | (part << 5) | rd);
    }
  }
}

static void a64_encode(context *ctx, char *mnemonic, WIDTH width, OPERAND oa,
                       uint64_t ia, rel ra, OPERAND ob, uint64_t ib, rel rb) {
  if (mnemonic[0] == '.')
    return;
  const a64_form *f = a64_find(mnemonic, oa, ob);
  if (!f) {
    masm_dief("arm64", "no form for '%s'", mnemonic);
  }
  static const uint8_t a64_regs[11] = {0, 1, 2, 3, 4, 5, 6, 7, 17, 31, 29};
  uint8_t rd = a64_regs[ia & 15];
  uint8_t rs = a64_regs[ib & 15];

  switch (f->kind) {
  case A64_FIXED:
    a64_emit(ctx, (uint32_t)f->base);
    break;
  case A64_SVC:
    if (oa == OPERAND_IMM)
      a64_emit(ctx, (uint32_t)f->base | (((uint32_t)ia & 0xFFFFu) << 5));
    else
      a64_emit(ctx, (uint32_t)f->base);
    break;
  case A64_MOV:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)rs << 16) | rd);
    break;
  case A64_UN:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)rd << 16) | rd);
    break;
  case A64_SXTW:
    a64_emit(ctx, (uint32_t)f->base | ((uint32_t)rs << 5) | rd);
    break;
  case A64_DP3:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)rs << 16) |
                       ((uint32_t)rd << 5) | rd);
    break;
  case A64_CC:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)rs << 16) |
                       ((uint32_t)rd << 5));
    break;
  case A64_DP1:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)rd << 5) | rd);
    break;
  case A64_MOV_I:
    a64_mov_imm(ctx, width, rd, ib);
    break;
  case A64_DP_RI:
  case A64_CC_RI: {
    int64_t v = (int64_t)ib;
    uint32_t u = (uint32_t)v;
    uint8_t rt = (f->kind == A64_CC_RI) ? A64_ZR : rd;
    if (v >= 0 && u <= 4095u) {
      a64_emit(ctx, a64_w(width, f->base) | (u << 10) | ((uint32_t)rd << 5) |
                         rt);
    } else if (f->kind == A64_CC_RI && v < 0 && -v <= 4095) {
      a64_emit(ctx, a64_w(width, f->base ^ 0x40000000ull) |
                         (((uint32_t)(-v)) << 10) | ((uint32_t)rd << 5) | rt);
    } else if (f->kind == A64_DP_RI && v < 0 &&
               (f->base & 0x40000000ull) && -v <= 4095) {
      a64_emit(ctx, a64_w(width, f->base ^ 0x40000000ull) |
                         (((uint32_t)(-v)) << 10) | ((uint32_t)rd << 5) | rt);
    } else {
      a64_mov_imm(ctx, a64_is32(width) ? WIDTH_DWORD : WIDTH_QWORD, A64_IP, ib);
      uint64_t reg = f->base ^ 0x1A000000ull;
      if (f->kind == A64_CC_RI)
        a64_emit(ctx, a64_w(width, reg) | ((uint32_t)A64_IP << 16) |
                           ((uint32_t)rd << 5));
      else
        a64_emit(ctx, a64_w(width, reg) | ((uint32_t)A64_IP << 16) |
                           ((uint32_t)rd << 5) | rd);
    }
    break;
  }
  case A64_MUL_RR:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)rs << 16) |
                       ((uint32_t)rd << 5) | rd);
    break;
  case A64_MUL_RI:
    a64_mov_imm(ctx, a64_is32(width) ? WIDTH_DWORD : WIDTH_QWORD, A64_IP, ib);
    a64_emit(ctx, a64_w(width, 0x9B007C00ull) | ((uint32_t)A64_IP << 16) |
                       ((uint32_t)rd << 5) | rd);
    break;
  case A64_DIV:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)rs << 16) |
                       ((uint32_t)rd << 5) | rd);
    break;
  case A64_SH_RR:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)rs << 16) |
                       ((uint32_t)rd << 5) | rd);
    break;
  case A64_ROL_RR:
    a64_emit(ctx, a64_w(width, 0xCB0003E0ull) | ((uint32_t)rs << 16) | A64_IP);
    a64_emit(ctx, a64_w(width, 0x9AC02C00ull) | ((uint32_t)A64_IP << 16) |
                       ((uint32_t)rd << 5) | rd);
    break;
  case A64_SH_I: {
    int bits = a64_is32(width) ? 32 : 64;
    uint32_t n = (uint32_t)ib & (uint32_t)(bits - 1);
    uint32_t imms = (uint32_t)bits - 1;
    if (f->ext == 0) {
      uint32_t immr = (uint32_t)((bits - (int)n) % bits);
      a64_emit(ctx, a64_bw(width, 0xD3400000ull) | (immr << 16) | (imms << 10) |
                         ((uint32_t)rd << 5) | rd);
    } else if (f->ext == 1) {
      a64_emit(ctx, a64_bw(width, 0xD3400000ull) | (n << 16) | (imms << 10) |
                         ((uint32_t)rd << 5) | rd);
    } else if (f->ext == 2) {
      a64_emit(ctx, a64_bw(width, 0x93400000ull) | (n << 16) | (imms << 10) |
                         ((uint32_t)rd << 5) | rd);
    } else {
      if (n == 0)
        break;
      a64_emit(ctx, a64_bw(width, f->base) | ((uint32_t)rd << 16) | (n << 10) |
                         ((uint32_t)rd << 5) | rd);
    }
    break;
  }
  case A64_ROL_I: {
    int bits = a64_is32(width) ? 32 : 64;
    uint32_t n = (uint32_t)ib & (uint32_t)(bits - 1);
    if (n == 0)
      break;
    a64_emit(ctx, a64_bw(width, f->base) | ((uint32_t)rd << 16) |
                       (((uint32_t)bits - n) << 10) | ((uint32_t)rd << 5) | rd);
    break;
  }
  case A64_PUSH:
    a64_emit(ctx, a64_w(width, f->base) | rd);
    break;
  case A64_POP:
    a64_emit(ctx, a64_w(width, f->base) | rd);
    break;
  case A64_BR: {
    uint8_t rtype = (f->base & 0x80000000ull) ? REL_BRCALL : REL_BRJMP;
    if (oa == OPERAND_REL && ra.section == SECTION_TEXT) {
      int64_t off = ((int64_t)ra.adrs - (int64_t)*ctx->current_text_adrs) >> 2;
      a64_emit(ctx, (uint32_t)f->base | ((uint32_t)off & 0x03FFFFFFu));
    } else if (oa == OPERAND_REL) {
      a64_rel(ctx, *ctx->current_text_adrs, ra.section, ra.name,
              (int64_t)ra.adrs, rtype);
      a64_emit(ctx, (uint32_t)f->base);
    } else {
      fprintf(stderr, "masm: arm64: '%s' needs a label operand\n", mnemonic);
      exit(1);
    }
    break;
  }
  case A64_BCC:
    if (oa == OPERAND_REL && ra.section == SECTION_TEXT) {
      int64_t off = ((int64_t)ra.adrs - (int64_t)*ctx->current_text_adrs) >> 2;
      a64_emit(ctx, (uint32_t)f->base | (((uint32_t)off & 0x7FFFFu) << 5) |
                         f->ext);
    } else if (oa == OPERAND_REL) {
      a64_rel(ctx, *ctx->current_text_adrs, ra.section, ra.name,
              (int64_t)ra.adrs, REL_BCC19);
      a64_emit(ctx, (uint32_t)f->base | f->ext);
    } else {
      fprintf(stderr, "masm: arm64: '%s' needs a label operand\n", mnemonic);
      exit(1);
    }
    break;
  case A64_SETCC:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)(f->ext ^ 1) << 12) | rd);
    break;
  case A64_CMOV:
    a64_emit(ctx, a64_w(width, f->base) | ((uint32_t)rs << 16) |
                       ((uint32_t)(f->ext ^ 1) << 12) | ((uint32_t)rs << 5) |
                       rd);
    break;
  case A64_ADR: {
    int local = (rb.section == SECTION_TEXT);
    int64_t off = 0;
    if (local) {
      off = (int64_t)rb.adrs - (int64_t)*ctx->current_text_adrs;
      if (off < -(1 << 20) || off >= (1 << 20))
        local = 0;
    }
    if (local) {
      uint32_t imm = (uint32_t)off;
      a64_emit(ctx, (uint32_t)f->base | ((imm & 3u) << 29) |
                         (((imm >> 2) & 0x7FFFFu) << 5) | rd);
    } else {
      a64_rel(ctx, *ctx->current_text_adrs, rb.section, rb.name,
              (int64_t)rb.adrs, REL_ADR21);
      a64_emit(ctx, (uint32_t)f->base | rd);
    }
    break;
  }
  case A64_MEM: {
    int is_load = f->ext;
    int local = (rb.section == SECTION_TEXT);
    int64_t off = 0;
    if (local) {
      off = (int64_t)rb.adrs - (int64_t)*ctx->current_text_adrs;
      if (off < -(1 << 20) || off >= (1 << 20))
        local = 0;
    }
    uint32_t ls;
    if (width == WIDTH_BYTE)
      ls = is_load ? 0x39400000u : 0x39000000u;
    else if (width == WIDTH_WORD)
      ls = is_load ? 0x79400000u : 0x79000000u;
    else if (width == WIDTH_DWORD)
      ls = is_load ? 0xB9400000u : 0xB9000000u;
    else
      ls = is_load ? 0xF9400000u : 0xF9000000u;
    uint8_t ab = local ? rd : A64_IP;
    if (local) {
      uint32_t imm = (uint32_t)off;
      a64_emit(ctx, 0x10000000u | ((imm & 3u) << 29) |
                         (((imm >> 2) & 0x7FFFFu) << 5) | rd);
    } else {
      a64_rel(ctx, *ctx->current_text_adrs, rb.section, rb.name,
              (int64_t)rb.adrs, REL_ADR21);
      a64_emit(ctx, 0x10000000u | A64_IP);
    }
    a64_emit(ctx, ls | ((uint32_t)ab << 5) | rd);
    break;
  }
  default:
    fprintf(stderr, "masm: arm64: '%s' not supported\n", mnemonic);
    exit(1);
  }
}

static registry_object arm64_object = {
    REGISTRY_ENCODER, "arm64", NULL, a64_encode, NULL,
};

static void arm64_register(void) __attribute__((constructor));
static void arm64_register(void) { register_object(&arm64_object); }

#endif
