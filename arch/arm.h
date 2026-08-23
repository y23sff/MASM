#ifndef MASM_ARM_H
#define MASM_ARM_H

#include "../core.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARM_SP 13
#define ARM_LR 14
#define ARM_PC 15
#define ARM_IP 12

#define ARM_ANY 0xFFu
#define ARM_TGT 0xFEu

enum {
  ARM_FIXED = 1,
  ARM_FIXED2,
  ARM_SVC,
  ARM_DP_RR,
  ARM_CC_RR,
  ARM_DP_RI,
  ARM_CC_RI,
  ARM_DP1,
  ARM_MVN,
  ARM_MOV_I,
  ARM_MUL_RR,
  ARM_MUL_RI,
  ARM_SDIV,
  ARM_SH_RR,
  ARM_SH_I,
  ARM_ROL_RR,
  ARM_ROL_I,
  ARM_XCHG,
  ARM_PUSH,
  ARM_POP,
  ARM_BR,
  ARM_SETCC,
  ARM_ADDR,
  ARM_LOAD,
  ARM_STORE
};

typedef struct {
  const char *mn;
  uint8_t oa, ob;
  uint8_t kind;
  uint32_t base;
} arm_form;

#define F(mn, oa, ob, k, b)                                                    \
  { mn, (uint8_t)(oa), (uint8_t)(ob), (uint8_t)(k), b }

static const arm_form arm_tab[] = {
    F("nop", OPERAND_NULL, OPERAND_NULL, ARM_FIXED, 0xE1A00000u),
    F("ret", OPERAND_NULL, OPERAND_NULL, ARM_FIXED, 0xE1A0F00Eu),
    F("leave", OPERAND_NULL, OPERAND_NULL, ARM_FIXED2, 0),
    F("syscall", OPERAND_NULL, OPERAND_NULL, ARM_SVC, 0xEF000000u),
    F("int", OPERAND_IMM, OPERAND_NULL, ARM_SVC, 0xEF000000u),

    F("push", OPERAND_REG, OPERAND_NULL, ARM_PUSH, 0xE52D0004u),
    F("pop", OPERAND_REG, OPERAND_NULL, ARM_POP, 0xE49D0004u),
    F("inc", OPERAND_REG, OPERAND_NULL, ARM_DP1, 0xE2800001u),
    F("dec", OPERAND_REG, OPERAND_NULL, ARM_DP1, 0xE2400001u),
    F("neg", OPERAND_REG, OPERAND_NULL, ARM_DP1, 0xE2600000u),
    F("not", OPERAND_REG, OPERAND_NULL, ARM_MVN, 0xE1E00000u),

    F("mov", OPERAND_REG, OPERAND_REG, ARM_DP_RR, 0xE1A00000u),
    F("mov", OPERAND_REG, OPERAND_IMM, ARM_MOV_I, 0),
    F("add", OPERAND_REG, OPERAND_REG, ARM_DP_RR, 0xE0800000u),
    F("add", OPERAND_REG, OPERAND_IMM, ARM_DP_RI, 0xE2800000u),
    F("sub", OPERAND_REG, OPERAND_REG, ARM_DP_RR, 0xE0400000u),
    F("sub", OPERAND_REG, OPERAND_IMM, ARM_DP_RI, 0xE2400000u),
    F("and", OPERAND_REG, OPERAND_REG, ARM_DP_RR, 0xE0000000u),
    F("and", OPERAND_REG, OPERAND_IMM, ARM_DP_RI, 0xE2000000u),
    F("or", OPERAND_REG, OPERAND_REG, ARM_DP_RR, 0xE1800000u),
    F("or", OPERAND_REG, OPERAND_IMM, ARM_DP_RI, 0xE3800000u),
    F("xor", OPERAND_REG, OPERAND_REG, ARM_DP_RR, 0xE0200000u),
    F("xor", OPERAND_REG, OPERAND_IMM, ARM_DP_RI, 0xE2200000u),
    F("cmp", OPERAND_REG, OPERAND_REG, ARM_CC_RR, 0xE1500000u),
    F("cmp", OPERAND_REG, OPERAND_IMM, ARM_CC_RI, 0xE3500000u),
    F("test", OPERAND_REG, OPERAND_REG, ARM_CC_RR, 0xE1100000u),
    F("test", OPERAND_REG, OPERAND_IMM, ARM_CC_RI, 0xE3100000u),
    F("adc", OPERAND_REG, OPERAND_REG, ARM_DP_RR, 0xE0A00000u),
    F("adc", OPERAND_REG, OPERAND_IMM, ARM_DP_RI, 0xE2A00000u),
    F("sbb", OPERAND_REG, OPERAND_REG, ARM_DP_RR, 0xE0C00000u),
    F("sbb", OPERAND_REG, OPERAND_IMM, ARM_DP_RI, 0xE2C00000u),
    F("mul", OPERAND_REG, OPERAND_REG, ARM_MUL_RR, 0),
    F("mul", OPERAND_REG, OPERAND_IMM, ARM_MUL_RI, 0),
    F("imul", OPERAND_REG, OPERAND_REG, ARM_MUL_RR, 0),
    F("imul", OPERAND_REG, OPERAND_IMM, ARM_MUL_RI, 0),
    F("div", OPERAND_REG, OPERAND_REG, ARM_SDIV, 0xE7100F10u),
    F("idiv", OPERAND_REG, OPERAND_REG, ARM_SDIV, 0xE7100F10u),
    F("xchg", OPERAND_REG, OPERAND_REG, ARM_XCHG, 0),

    F("shl", OPERAND_REG, OPERAND_REG, ARM_SH_RR, 0xE1A00010u),
    F("shl", OPERAND_REG, OPERAND_IMM, ARM_SH_I, 0xE1A00000u),
    F("sal", OPERAND_REG, OPERAND_REG, ARM_SH_RR, 0xE1A00010u),
    F("sal", OPERAND_REG, OPERAND_IMM, ARM_SH_I, 0xE1A00000u),
    F("shr", OPERAND_REG, OPERAND_REG, ARM_SH_RR, 0xE1A00030u),
    F("shr", OPERAND_REG, OPERAND_IMM, ARM_SH_I, 0xE1A00020u),
    F("sar", OPERAND_REG, OPERAND_REG, ARM_SH_RR, 0xE1A00050u),
    F("sar", OPERAND_REG, OPERAND_IMM, ARM_SH_I, 0xE1A00040u),
    F("ror", OPERAND_REG, OPERAND_REG, ARM_SH_RR, 0xE1A00070u),
    F("ror", OPERAND_REG, OPERAND_IMM, ARM_SH_I, 0xE1A00060u),
    F("rol", OPERAND_REG, OPERAND_REG, ARM_ROL_RR, 0),
    F("rol", OPERAND_REG, OPERAND_IMM, ARM_ROL_I, 0xE1A00060u),

    F("jmp", ARM_TGT, OPERAND_NULL, ARM_BR, 0xEA000000u),
    F("b", ARM_TGT, OPERAND_NULL, ARM_BR, 0xEA000000u),
    F("call", ARM_TGT, OPERAND_NULL, ARM_BR, 0xEB000000u),
    F("bl", ARM_TGT, OPERAND_NULL, ARM_BR, 0xEB000000u),

    F("jo", ARM_TGT, OPERAND_NULL, ARM_BR, 0x3A000000u),
    F("jno", ARM_TGT, OPERAND_NULL, ARM_BR, 0x1A000000u),
    F("je", ARM_TGT, OPERAND_NULL, ARM_BR, 0x0A000000u),
    F("jz", ARM_TGT, OPERAND_NULL, ARM_BR, 0x0A000000u),
    F("jne", ARM_TGT, OPERAND_NULL, ARM_BR, 0x1A000000u),
    F("jnz", ARM_TGT, OPERAND_NULL, ARM_BR, 0x1A000000u),
    F("jb", ARM_TGT, OPERAND_NULL, ARM_BR, 0x3A000000u),
    F("jc", ARM_TGT, OPERAND_NULL, ARM_BR, 0x3A000000u),
    F("jnae", ARM_TGT, OPERAND_NULL, ARM_BR, 0x3A000000u),
    F("jae", ARM_TGT, OPERAND_NULL, ARM_BR, 0x2A000000u),
    F("jnb", ARM_TGT, OPERAND_NULL, ARM_BR, 0x2A000000u),
    F("jnc", ARM_TGT, OPERAND_NULL, ARM_BR, 0x2A000000u),
    F("jbe", ARM_TGT, OPERAND_NULL, ARM_BR, 0x9A000000u),
    F("jna", ARM_TGT, OPERAND_NULL, ARM_BR, 0x9A000000u),
    F("ja", ARM_TGT, OPERAND_NULL, ARM_BR, 0x8A000000u),
    F("jnbe", ARM_TGT, OPERAND_NULL, ARM_BR, 0x8A000000u),
    F("js", ARM_TGT, OPERAND_NULL, ARM_BR, 0x4A000000u),
    F("jns", ARM_TGT, OPERAND_NULL, ARM_BR, 0x5A000000u),
    F("jl", ARM_TGT, OPERAND_NULL, ARM_BR, 0xBA000000u),
    F("jnge", ARM_TGT, OPERAND_NULL, ARM_BR, 0xBA000000u),
    F("jge", ARM_TGT, OPERAND_NULL, ARM_BR, 0xAA000000u),
    F("jnl", ARM_TGT, OPERAND_NULL, ARM_BR, 0xAA000000u),
    F("jle", ARM_TGT, OPERAND_NULL, ARM_BR, 0xDA000000u),
    F("jng", ARM_TGT, OPERAND_NULL, ARM_BR, 0xDA000000u),
    F("jg", ARM_TGT, OPERAND_NULL, ARM_BR, 0xCA000000u),
    F("jnle", ARM_TGT, OPERAND_NULL, ARM_BR, 0xCA000000u),

    F("seto", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x33A00000u),
    F("setno", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x13A00000u),
    F("sete", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x03A00000u),
    F("setz", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x03A00000u),
    F("setne", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x13A00000u),
    F("setnz", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x13A00000u),
    F("setb", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x33A00000u),
    F("setc", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x33A00000u),
    F("setnae", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x33A00000u),
    F("setae", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x23A00000u),
    F("setnb", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x23A00000u),
    F("setnc", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x23A00000u),
    F("setbe", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x93A00000u),
    F("setna", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x93A00000u),
    F("seta", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x83A00000u),
    F("setnbe", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x83A00000u),
    F("sets", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x43A00000u),
    F("setns", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0x53A00000u),
    F("setl", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0xB3A00000u),
    F("setnge", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0xB3A00000u),
    F("setge", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0xA3A00000u),
    F("setnl", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0xA3A00000u),
    F("setle", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0xD3A00000u),
    F("setng", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0xD3A00000u),
    F("setg", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0xC3A00000u),
    F("setnle", OPERAND_REG, OPERAND_NULL, ARM_SETCC, 0xC3A00000u),

    F("ldr", OPERAND_REG, OPERAND_REL, ARM_ADDR, 0),
    F("lea", OPERAND_REG, OPERAND_REL, ARM_ADDR, 0),
    F("load", OPERAND_REG, OPERAND_REL, ARM_LOAD, 0),
    F("str", OPERAND_REG, OPERAND_REL, ARM_STORE, 0),
};

#undef F

static void arm_emit(context *ctx, uint32_t w) {
  uint8_t b[4] = {(uint8_t)w, (uint8_t)(w >> 8), (uint8_t)(w >> 16),
                  (uint8_t)(w >> 24)};
  if (ctx->text_stream && fwrite(b, 1, 4, ctx->text_stream) != 4) {
    fprintf(stderr, "masm: arm: text stream full\n");
    exit(1);
  }
  *ctx->current_text_adrs += 4;
}

static void arm_pool(context *ctx, uint32_t v, SECTION sec, const char *name,
                     int64_t addend, int use_rel) {
  if (use_rel && ctx->text_stream) {
    rel *rr = ctx_new_rel(ctx);
    rr->adrs = *ctx->current_text_adrs;
    rr->section = sec;
    rr->name = name;
    rr->addend = addend;
    rr->type = REL_ABS32;
  }
  arm_emit(ctx, v);
}

static int arm_rot_imm(uint64_t v, uint32_t *imm12) {
  uint32_t x = (uint32_t)v;
  if (v > 0xFFFFFFFFull)
    return 0;
  for (int rot = 0; rot < 16; rot++) {
    uint32_t r = (x >> (2 * rot)) | (x << ((32 - 2 * rot) & 31));
    if (r <= 0xFF) {
      *imm12 = ((uint32_t)rot << 8) | r;
      return 1;
    }
  }
  return 0;
}

static int arm_match(uint8_t want, OPERAND got) {
  if (want == ARM_ANY)
    return 1;
  if (want == ARM_TGT)
    return got == OPERAND_REL || got == OPERAND_IMM;
  return want == (uint8_t)got;
}

static const arm_form *arm_find(const char *mn, OPERAND oa, OPERAND ob) {
  for (size_t i = 0; i < sizeof arm_tab / sizeof arm_tab[0]; i++) {
    const arm_form *f = &arm_tab[i];
    if (strcmp(f->mn, mn) != 0)
      continue;
    if (arm_match(f->oa, oa) && arm_match(f->ob, ob))
      return f;
  }
  return NULL;
}

static void arm_encode(context *ctx, char *mnemonic, WIDTH width, OPERAND oa,
                       uint64_t ia, rel ra, OPERAND ob, uint64_t ib, rel rb) {
  if (mnemonic[0] == '.')
    return;
  const arm_form *f = arm_find(mnemonic, oa, ob);
  if (!f) {
    masm_dief("arm", "no form for '%s'", mnemonic);
  }
  static const uint8_t arm_regs[11] = {0, 1, 2, 3, 4, 5, 6, 7, 12, 13, 11};
  uint8_t rd = arm_regs[ia & 15];
  uint8_t rs = arm_regs[ib & 15];

  switch (f->kind) {
  case ARM_FIXED:
    arm_emit(ctx, f->base);
    break;
  case ARM_FIXED2:
    arm_emit(ctx, 0xE1A00000u | ((uint32_t)ARM_SP << 12) | 5u);
    arm_emit(ctx, 0xE49D5004u);
    break;
  case ARM_SVC:
    arm_emit(ctx, f->base | ((uint32_t)ia & 0x00FFFFFFu));
    break;
  case ARM_DP_RR:
    arm_emit(ctx, f->base | ((uint32_t)rd << 16) | ((uint32_t)rd << 12) | rs);
    break;
  case ARM_CC_RR:
    arm_emit(ctx, f->base | ((uint32_t)rd << 16) | rs);
    break;
  case ARM_DP1:
    arm_emit(ctx, f->base | ((uint32_t)rd << 16) | ((uint32_t)rd << 12));
    break;
  case ARM_MVN:
    arm_emit(ctx, f->base | ((uint32_t)rd << 12) | rd);
    break;
  case ARM_DP_RI:
  case ARM_CC_RI: {
    uint32_t imm12;
    uint8_t d = (f->kind == ARM_CC_RI) ? 0 : rd;
    if (arm_rot_imm(ib, &imm12)) {
      arm_emit(ctx, f->base | ((uint32_t)rd << 16) | ((uint32_t)d << 12) |
                         imm12);
    } else {
      arm_emit(ctx, 0xE59FC000u);
      arm_emit(ctx, (f->base & ~(1u << 25)) | ((uint32_t)rd << 16) |
                         ((uint32_t)d << 12) | ARM_IP);
      arm_pool(ctx, (uint32_t)ib, 0, 0, 0, 0);
    }
    break;
  }
  case ARM_MOV_I:
    arm_emit(ctx, 0xE51F0000u | ((uint32_t)rd << 12) | 4u);
    arm_pool(ctx, (uint32_t)ib, 0, 0, 0, 0);
    break;
  case ARM_MUL_RR:
    arm_emit(ctx, 0xE0000090u | ((uint32_t)ARM_IP << 16) |
                       ((uint32_t)rs << 8) | rd);
    arm_emit(ctx, 0xE1A00000u | ((uint32_t)rd << 12) | ARM_IP);
    break;
  case ARM_MUL_RI:
    arm_emit(ctx, 0xE59FC000u);
    arm_emit(ctx, 0xE0000090u | ((uint32_t)ARM_IP << 16) |
                       ((uint32_t)ARM_IP << 8) | rd);
    arm_pool(ctx, (uint32_t)ib, 0, 0, 0, 0);
    arm_emit(ctx, 0xE1A00000u | ((uint32_t)rd << 12) | ARM_IP);
    break;
  case ARM_SDIV:
    arm_emit(ctx, f->base | ((uint32_t)rd << 16) | ((uint32_t)rd << 12) | rs);
    break;
  case ARM_SH_RR:
    arm_emit(ctx, f->base | ((uint32_t)rs << 8) | ((uint32_t)rd << 12) | rd);
    break;
  case ARM_SH_I: {
    if ((int64_t)ib < 0 || (int64_t)ib > 31) {
      masm_dief("arm", "shift count out of range");
    }
    uint32_t n = (uint32_t)ib & 31u;
    if (n == 0 && (f->base & 0x60u) == 0x60u)
      break;
    arm_emit(ctx, f->base | (n << 7) | rd);
    break;
  }
  case ARM_ROL_RR:
    arm_emit(ctx, 0xE2600000u | ((uint32_t)rs << 16) |
                       ((uint32_t)ARM_IP << 12) | 32u);
    arm_emit(ctx, 0xE1A00070u | ((uint32_t)ARM_IP << 8) |
                       ((uint32_t)rd << 12) | rd);
    break;
  case ARM_ROL_I: {
    if ((int64_t)ib < 0 || (int64_t)ib > 31) {
      masm_dief("arm", "shift count out of range");
    }
    uint32_t n = (uint32_t)ib & 31u;
    if (n == 0)
      break;
    arm_emit(ctx, f->base | ((32u - n) << 7) | rd);
    break;
  }
  case ARM_XCHG:
    arm_emit(ctx, 0xE1A00000u | ((uint32_t)ARM_IP << 12) | rd);
    arm_emit(ctx, 0xE1A00000u | ((uint32_t)rd << 12) | rs);
    arm_emit(ctx, 0xE1A00000u | ((uint32_t)rs << 12) | ARM_IP);
    break;
  case ARM_PUSH:
    arm_emit(ctx, f->base | ((uint32_t)rd << 12));
    break;
  case ARM_POP:
    arm_emit(ctx, f->base | ((uint32_t)rd << 12));
    break;
  case ARM_BR: {
    uint8_t rtype = (f->base & 0x01000000u) ? REL_BRCALL : REL_BRJMP;
    int64_t imm;
    if (oa == OPERAND_IMM) {
      imm = ((int64_t)(int32_t)ia) >> 2;
    } else if (ra.section == SECTION_TEXT) {
      int64_t off = (int64_t)ra.adrs - (int64_t)*ctx->current_text_adrs - 8;
      imm = off >> 2;
    } else if (ra.section == SECTION_NULL && ra.name) {
      if (ctx->text_stream) {
        rel *rr = ctx_new_rel(ctx);
        rr->adrs = *ctx->current_text_adrs;
        rr->section = SECTION_NULL;
        rr->name = ra.name;
        rr->addend = (int64_t)ra.adrs - 8;
        rr->type = rtype;
      }
      imm = 0;
    } else {
      fprintf(stderr, "masm: arm: branch target must be a .text label ('%s')\n",
              mnemonic);
      exit(1);
    }
    if (imm < -(1 << 23) || imm >= (1 << 23)) {
      fprintf(stderr, "masm: arm: branch out of range ('%s')\n", mnemonic);
      exit(1);
    }
    arm_emit(ctx, f->base | ((uint32_t)imm & 0x00FFFFFFu));
    break;
  }
  case ARM_SETCC:
    arm_emit(ctx, 0xE3A00000u | ((uint32_t)rd << 12));
    arm_emit(ctx, f->base | ((uint32_t)rd << 12) | 1u);
    break;
  case ARM_ADDR:
    arm_emit(ctx, 0xE51F0000u | ((uint32_t)rd << 12) | 4u);
    arm_pool(ctx, 0, rb.section, rb.name, (int64_t)rb.adrs, 1);
    break;
  case ARM_LOAD:
    arm_emit(ctx, 0xE51F0000u | ((uint32_t)rd << 12) | 4u);
    arm_pool(ctx, 0, rb.section, rb.name, (int64_t)rb.adrs, 1);
    if (width == WIDTH_BYTE)
      arm_emit(ctx, 0xE5D00000u | ((uint32_t)rd << 16) | ((uint32_t)rd << 12));
    else
      arm_emit(ctx, 0xE5900000u | ((uint32_t)rd << 16) | ((uint32_t)rd << 12));
    break;
  case ARM_STORE:
    arm_emit(ctx, 0xE59FC000u);
    if (width == WIDTH_BYTE)
      arm_emit(ctx, 0xE5CC0000u | ((uint32_t)rd << 12));
    else
      arm_emit(ctx, 0xE580C000u | ((uint32_t)rd << 12));
    arm_pool(ctx, 0, rb.section, rb.name, (int64_t)rb.adrs, 1);
    break;
  default:
    fprintf(stderr, "masm: arm: encode failed for '%s'\n", mnemonic);
    exit(1);
  }
}

static registry_object arm_object = {
    REGISTRY_ENCODER, "arm", NULL, arm_encode, NULL,
};

static void arm_register(void) __attribute__((constructor));
static void arm_register(void) { register_object(&arm_object); }

#endif
