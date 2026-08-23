#ifndef MASM_X86_H
#define MASM_X86_H

#include "../core.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t x64_code[11] = {0, 7, 6, 2, 1, 8, 9, 10, 11, 4, 5};
static const uint8_t x32_code[11] = {0, 7, 6, 2, 1, 5, 3, 4, 1, 4, 5};

static int x64_mode64 = 1;

#define X64_ANY  0xFFu
#define X64_RI   0xFEu

#define X64_W_N  (1u << WIDTH_NULL)
#define X64_W_B  (1u << WIDTH_BYTE)
#define X64_W_W  (1u << WIDTH_WORD)
#define X64_W_D  (1u << WIDTH_DWORD)
#define X64_W_Q  (1u << WIDTH_QWORD)
#define X64_W_ALL (X64_W_N | X64_W_B | X64_W_W | X64_W_D | X64_W_Q)
#define X64_W_INT (X64_W_N | X64_W_W | X64_W_D | X64_W_Q)
#define X64_W_NQ  (X64_W_N | X64_W_Q)

enum {
  X64_NP = 1,
  X64_PLUSR,
  X64_MOVIMM,
  X64_RM,
  X64_MR,
  X64_EXT,
  X64_EXTIMM,
  X64_SHIFT,
  X64_REL32,
  X64_JCC,
  X64_SETCC,
  X64_LEA,
  X64_LOAD,
  X64_STORE,
  X64_INT,
  X64_IMUL3
};

typedef struct {
  const char *mn;
  uint8_t oa, ob;
  uint8_t wmask;
  uint8_t kind;
  uint8_t op[4];
  uint8_t oplen;
  uint8_t ext;
} x64_form;

#define F(mn, oa, ob, wm, k, e, n, ...)                                        \
  { mn, (uint8_t)(oa), (uint8_t)(ob), (uint8_t)(wm), (uint8_t)(k),             \
    {__VA_ARGS__}, n, e }

static const x64_form x64_tab[] = {
    F("nop", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x90),
    F("ret", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xC3),
    F("retf", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xCB),
    F("syscall", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x0F, 0x05),
    F("sysret", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x0F, 0x07),
    F("sysenter", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x0F, 0x34),
    F("sysexit", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x0F, 0x35),
    F("int3", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xCC),
    F("ud2", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x0F, 0x0B),
    F("hlt", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xF4),
    F("cli", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xFA),
    F("sti", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xFB),
    F("cld", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xFC),
    F("std", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xFD),
    F("clc", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xF8),
    F("stc", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xF9),
    F("cmc", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xF5),
    F("cbw", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x66, 0x98),
    F("cwde", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x98),
    F("cdqe", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x48, 0x98),
    F("cwd", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x66, 0x99),
    F("cdq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x99),
    F("cqo", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x48, 0x99),
    F("lahf", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x9F),
    F("sahf", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x9E),
    F("pushfq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x9C),
    F("popfq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x9D),
    F("pushf", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x9C),
    F("popf", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x9D),
    F("leave", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xC9),
    F("iret", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xCF),
    F("iretd", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xCF),
    F("iretq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x48, 0xCF),
    F("cpuid", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x0F, 0xA2),
    F("pause", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0xF3, 0x90),
    F("lfence", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 3, 0x0F, 0xAE, 0xE8),
    F("sfence", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 3, 0x0F, 0xAE, 0xF8),
    F("mfence", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 3, 0x0F, 0xAE, 0xF0),
    F("rdtsc", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x0F, 0x31),
    F("rdtscp", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 3, 0x0F, 0x01, 0xF9),
    F("rdpmc", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x0F, 0x33),
    F("xgetbv", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 3, 0x0F, 0x01, 0xD0),
    F("endbr64", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 4, 0xF3, 0x0F, 0x1E, 0xFA),
    F("endbr32", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 4, 0xF3, 0x0F, 0x1E, 0xFB),
    F("xlat", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xD7),
    F("wait", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x9B),
    F("fwait", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0x9B),
    F("emms", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x0F, 0x77),
    F("serialize", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 3, 0x0F, 0x01, 0xE8),
    F("movsb", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xA4),
    F("movsw", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x66, 0xA5),
    F("movsd", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xA5),
    F("movsq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x48, 0xA5),
    F("stosb", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xAA),
    F("stosw", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x66, 0xAB),
    F("stosd", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xAB),
    F("stosq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x48, 0xAB),
    F("lodsb", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xAC),
    F("lodsw", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x66, 0xAD),
    F("lodsd", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xAD),
    F("lodsq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x48, 0xAD),
    F("scasb", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xAE),
    F("scasw", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x66, 0xAF),
    F("scasd", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xAF),
    F("scasq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x48, 0xAF),
    F("cmpsb", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xA6),
    F("cmpsw", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x66, 0xA7),
    F("cmpsd", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 1, 0xA7),
    F("cmpsq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0x48, 0xA7),
    F("rep_movsb", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0xF3, 0xA4),
    F("rep_movsq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 3, 0xF3, 0x48, 0xA5),
    F("rep_stosb", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 2, 0xF3, 0xAA),
    F("rep_stosq", OPERAND_NULL, OPERAND_NULL, X64_W_ALL, X64_NP, 0, 3, 0xF3, 0x48, 0xAB),
    F("push", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_PLUSR, 0, 1, 0x50),
    F("pop", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_PLUSR, 0, 1, 0x58),
    F("bswap", OPERAND_REG, OPERAND_NULL, X64_W_INT, X64_PLUSR, 0, 2, 0x0F, 0xC8),
    F("mov", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_MOVIMM, 0, 1, 0xB8),
    F("mov", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x89),
    F("add", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x01),
    F("or", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x09),
    F("adc", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x11),
    F("sbb", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x19),
    F("and", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x21),
    F("sub", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x29),
    F("xor", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x31),
    F("cmp", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x39),
    F("test", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x85),
    F("xchg", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 1, 0x87),
    F("xadd", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 2, 0x0F, 0xC1),
    F("cmpxchg", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_RM, 0, 2, 0x0F, 0xB1),
    F("add", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_EXTIMM, 0, 1, 0x81),
    F("or", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_EXTIMM, 1, 1, 0x81),
    F("adc", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_EXTIMM, 2, 1, 0x81),
    F("sbb", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_EXTIMM, 3, 1, 0x81),
    F("and", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_EXTIMM, 4, 1, 0x81),
    F("sub", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_EXTIMM, 5, 1, 0x81),
    F("xor", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_EXTIMM, 6, 1, 0x81),
    F("cmp", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_EXTIMM, 7, 1, 0x81),
    F("test", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_EXTIMM, 0, 1, 0xF7),
    F("inc", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_EXT, 0, 1, 0xFF),
    F("dec", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_EXT, 1, 1, 0xFF),
    F("not", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_EXT, 2, 1, 0xF7),
    F("neg", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_EXT, 3, 1, 0xF7),
    F("mul", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_EXT, 4, 1, 0xF7),
    F("div", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_EXT, 6, 1, 0xF7),
    F("idiv", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_EXT, 7, 1, 0xF7),
    F("imul", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_EXT, 5, 1, 0xF7),
    F("imul", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 0, 2, 0x0F, 0xAF),
    F("imul", OPERAND_REG, OPERAND_IMM, X64_W_INT, X64_IMUL3, 0, 1, 0x69),
    F("rol", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_SHIFT, 0, 1, 0xC1),
    F("ror", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_SHIFT, 1, 1, 0xC1),
    F("rcl", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_SHIFT, 2, 1, 0xC1),
    F("rcr", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_SHIFT, 3, 1, 0xC1),
    F("shl", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_SHIFT, 4, 1, 0xC1),
    F("sal", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_SHIFT, 4, 1, 0xC1),
    F("shr", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_SHIFT, 5, 1, 0xC1),
    F("sar", OPERAND_REG, OPERAND_IMM, X64_W_ALL, X64_SHIFT, 7, 1, 0xC1),
    F("rol", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_SHIFT, 0, 1, 0xD3),
    F("ror", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_SHIFT, 1, 1, 0xD3),
    F("rcl", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_SHIFT, 2, 1, 0xD3),
    F("rcr", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_SHIFT, 3, 1, 0xD3),
    F("shl", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_SHIFT, 4, 1, 0xD3),
    F("sal", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_SHIFT, 4, 1, 0xD3),
    F("shr", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_SHIFT, 5, 1, 0xD3),
    F("sar", OPERAND_REG, OPERAND_REG, X64_W_ALL, X64_SHIFT, 7, 1, 0xD3),
    F("bt", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_RM, 0, 2, 0x0F, 0xA3),
    F("bts", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_RM, 0, 2, 0x0F, 0xAB),
    F("btr", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_RM, 0, 2, 0x0F, 0xB3),
    F("btc", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_RM, 0, 2, 0x0F, 0xBB),
    F("bt", OPERAND_REG, OPERAND_IMM, X64_W_INT, X64_EXTIMM, 4, 2, 0x0F, 0xBA),
    F("bts", OPERAND_REG, OPERAND_IMM, X64_W_INT, X64_EXTIMM, 5, 2, 0x0F, 0xBA),
    F("btr", OPERAND_REG, OPERAND_IMM, X64_W_INT, X64_EXTIMM, 6, 2, 0x0F, 0xBA),
    F("btc", OPERAND_REG, OPERAND_IMM, X64_W_INT, X64_EXTIMM, 7, 2, 0x0F, 0xBA),
    F("bsf", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 0, 2, 0x0F, 0xBC),
    F("bsr", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 0, 2, 0x0F, 0xBD),
    F("tzcnt", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 0, 3, 0xF3, 0x0F, 0xBC),
    F("lzcnt", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 0, 3, 0xF3, 0x0F, 0xBD),
    F("popcnt", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 0, 3, 0xF3, 0x0F, 0xB8),
    F("movzx", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 0, 2, 0x0F, 0xB6),
    F("movsx", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 0, 2, 0x0F, 0xBE),
    F("movsxd", OPERAND_REG, OPERAND_REG, X64_W_NQ, X64_MR, 0, 1, 0x63),
    F("jmp", X64_RI, OPERAND_NULL, X64_W_ALL, X64_REL32, 0, 1, 0xE9),
    F("call", X64_RI, OPERAND_NULL, X64_W_ALL, X64_REL32, 0, 1, 0xE8),
    F("int", OPERAND_IMM, OPERAND_NULL, X64_W_ALL, X64_INT, 0, 1, 0xCD),
    F("lea", OPERAND_REG, X64_RI, X64_W_INT, X64_LEA, 0, 1, 0x8D),
    F("ldr", OPERAND_REG, X64_RI, X64_W_INT, X64_LEA, 0, 1, 0x8D),
    F("load", OPERAND_REG, X64_RI, X64_W_ALL, X64_LOAD, 0, 1, 0x8B),
    F("str", OPERAND_REG, X64_RI, X64_W_ALL, X64_STORE, 0, 1, 0x89),
    F("jo", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 0, 2, 0x0F, 0x80),
    F("jno", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 1, 2, 0x0F, 0x80),
    F("jb", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 2, 2, 0x0F, 0x80),
    F("jc", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 2, 2, 0x0F, 0x80),
    F("jnae", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 2, 2, 0x0F, 0x80),
    F("jae", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 3, 2, 0x0F, 0x80),
    F("jnb", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 3, 2, 0x0F, 0x80),
    F("jnc", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 3, 2, 0x0F, 0x80),
    F("je", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 4, 2, 0x0F, 0x80),
    F("jz", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 4, 2, 0x0F, 0x80),
    F("jne", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 5, 2, 0x0F, 0x80),
    F("jnz", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 5, 2, 0x0F, 0x80),
    F("jbe", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 6, 2, 0x0F, 0x80),
    F("jna", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 6, 2, 0x0F, 0x80),
    F("ja", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 7, 2, 0x0F, 0x80),
    F("jnbe", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 7, 2, 0x0F, 0x80),
    F("js", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 8, 2, 0x0F, 0x80),
    F("jns", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 9, 2, 0x0F, 0x80),
    F("jp", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 10, 2, 0x0F, 0x80),
    F("jpe", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 10, 2, 0x0F, 0x80),
    F("jnp", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 11, 2, 0x0F, 0x80),
    F("jpo", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 11, 2, 0x0F, 0x80),
    F("jl", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 12, 2, 0x0F, 0x80),
    F("jnge", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 12, 2, 0x0F, 0x80),
    F("jge", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 13, 2, 0x0F, 0x80),
    F("jnl", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 13, 2, 0x0F, 0x80),
    F("jle", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 14, 2, 0x0F, 0x80),
    F("jng", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 14, 2, 0x0F, 0x80),
    F("jg", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 15, 2, 0x0F, 0x80),
    F("jnle", X64_RI, OPERAND_NULL, X64_W_ALL, X64_JCC, 15, 2, 0x0F, 0x80),
    F("seto", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 0, 2, 0x0F, 0x90),
    F("setno", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 1, 2, 0x0F, 0x90),
    F("setb", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 2, 2, 0x0F, 0x90),
    F("setc", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 2, 2, 0x0F, 0x90),
    F("setnae", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 2, 2, 0x0F, 0x90),
    F("setae", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 3, 2, 0x0F, 0x90),
    F("setnb", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 3, 2, 0x0F, 0x90),
    F("setnc", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 3, 2, 0x0F, 0x90),
    F("sete", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 4, 2, 0x0F, 0x90),
    F("setz", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 4, 2, 0x0F, 0x90),
    F("setne", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 5, 2, 0x0F, 0x90),
    F("setnz", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 5, 2, 0x0F, 0x90),
    F("setbe", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 6, 2, 0x0F, 0x90),
    F("setna", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 6, 2, 0x0F, 0x90),
    F("seta", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 7, 2, 0x0F, 0x90),
    F("setnbe", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 7, 2, 0x0F, 0x90),
    F("sets", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 8, 2, 0x0F, 0x90),
    F("setns", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 9, 2, 0x0F, 0x90),
    F("setp", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 10, 2, 0x0F, 0x90),
    F("setpe", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 10, 2, 0x0F, 0x90),
    F("setnp", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 11, 2, 0x0F, 0x90),
    F("setpo", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 11, 2, 0x0F, 0x90),
    F("setl", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 12, 2, 0x0F, 0x90),
    F("setnge", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 12, 2, 0x0F, 0x90),
    F("setge", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 13, 2, 0x0F, 0x90),
    F("setnl", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 13, 2, 0x0F, 0x90),
    F("setle", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 14, 2, 0x0F, 0x90),
    F("setng", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 14, 2, 0x0F, 0x90),
    F("setg", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 15, 2, 0x0F, 0x90),
    F("setnle", OPERAND_REG, OPERAND_NULL, X64_W_ALL, X64_SETCC, 15, 2, 0x0F, 0x90),
    F("cmovo", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 0, 2, 0x0F, 0x40),
    F("cmovno", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 1, 2, 0x0F, 0x40),
    F("cmovb", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 2, 2, 0x0F, 0x40),
    F("cmovc", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 2, 2, 0x0F, 0x40),
    F("cmovnae", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 2, 2, 0x0F, 0x40),
    F("cmovae", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 3, 2, 0x0F, 0x40),
    F("cmovnb", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 3, 2, 0x0F, 0x40),
    F("cmovnc", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 3, 2, 0x0F, 0x40),
    F("cmove", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 4, 2, 0x0F, 0x40),
    F("cmovz", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 4, 2, 0x0F, 0x40),
    F("cmovne", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 5, 2, 0x0F, 0x40),
    F("cmovnz", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 5, 2, 0x0F, 0x40),
    F("cmovbe", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 6, 2, 0x0F, 0x40),
    F("cmovna", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 6, 2, 0x0F, 0x40),
    F("cmova", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 7, 2, 0x0F, 0x40),
    F("cmovnbe", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 7, 2, 0x0F, 0x40),
    F("cmovs", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 8, 2, 0x0F, 0x40),
    F("cmovns", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 9, 2, 0x0F, 0x40),
    F("cmovp", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 10, 2, 0x0F, 0x40),
    F("cmovpe", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 10, 2, 0x0F, 0x40),
    F("cmovnp", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 11, 2, 0x0F, 0x40),
    F("cmovpo", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 11, 2, 0x0F, 0x40),
    F("cmovl", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 12, 2, 0x0F, 0x40),
    F("cmovnge", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 12, 2, 0x0F, 0x40),
    F("cmovge", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 13, 2, 0x0F, 0x40),
    F("cmovnl", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 13, 2, 0x0F, 0x40),
    F("cmovle", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 14, 2, 0x0F, 0x40),
    F("cmovng", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 14, 2, 0x0F, 0x40),
    F("cmovg", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 15, 2, 0x0F, 0x40),
    F("cmovnle", OPERAND_REG, OPERAND_REG, X64_W_INT, X64_MR, 15, 2, 0x0F, 0x40),
};

#undef F

typedef struct {
  uint8_t *p;
  int n, cap;
} x64_w;

static void x64_b(x64_w *w, uint8_t v) {
  if (w->p && w->n < w->cap)
    w->p[w->n] = v;
  w->n++;
}

static void x64_n(x64_w *w, const uint8_t *s, int n) {
  for (int i = 0; i < n; i++)
    x64_b(w, s[i]);
}

static void x64_i32(x64_w *w, uint32_t v) {
  x64_b(w, (uint8_t)v);
  x64_b(w, (uint8_t)(v >> 8));
  x64_b(w, (uint8_t)(v >> 16));
  x64_b(w, (uint8_t)(v >> 24));
}

static void x64_i64(x64_w *w, uint64_t v) {
  for (int i = 0; i < 8; i++)
    x64_b(w, (uint8_t)(v >> (8 * i)));
}

static uint8_t x64_rex(int W, int R, int X, int B) {
  return (uint8_t)(0x40 | (W ? 8 : 0) | (R ? 4 : 0) | (X ? 2 : 0) | (B ? 1 : 0));
}

static uint8_t x64_modrm(uint8_t mod, uint8_t reg, uint8_t rm) {
  return (uint8_t)((mod << 6) | ((reg & 7) << 3) | (rm & 7));
}

static int x64_is_q(WIDTH w) {
  return x64_mode64 && (w == WIDTH_QWORD || w == WIDTH_NULL);
}
static int x64_is_b(WIDTH w) { return w == WIDTH_BYTE; }
static int x64_is_w(WIDTH w) { return w == WIDTH_WORD; }

static uint8_t x64_rc(uint64_t idx) {
  const uint8_t *t = x64_mode64 ? x64_code : x32_code;
  return (idx < 11) ? t[idx] : 0;
}

static int x64_hi(uint8_t c) { return c >= 8; }

static int x64_rex8(WIDTH w, uint8_t c) {
  return x64_mode64 && x64_is_b(w) && c >= 4;
}

static void x64_pfx(x64_w *w, WIDTH width, int rexw, int r, int b, uint8_t ca,
                    uint8_t cb) {
  if (x64_is_w(width))
    x64_b(w, 0x66);
  if (!x64_mode64)
    return;
  int need = rexw || r || b || x64_rex8(width, ca) || x64_rex8(width, cb);
  if (need)
    x64_b(w, x64_rex(rexw, r, 0, b));
}

static int x64_match_op(uint8_t want, OPERAND got) {
  if (want == X64_ANY)
    return 1;
  if (want == X64_RI)
    return got == OPERAND_REL || got == OPERAND_IMM;
  return want == (uint8_t)got;
}

static const x64_form *x64_find(const char *mn, OPERAND oa, OPERAND ob,
                                WIDTH width) {
  for (size_t i = 0; i < sizeof x64_tab / sizeof x64_tab[0]; i++) {
    const x64_form *f = &x64_tab[i];
    if (strcmp(f->mn, mn) != 0)
      continue;
    if (!x64_match_op(f->oa, oa) || !x64_match_op(f->ob, ob))
      continue;
    if (f->wmask & (1u << width))
      return f;
  }
  return NULL;
}

static int x64_imm8(int64_t v) { return v >= -128 && v <= 127; }

typedef struct {
  int use;
  int at;
  SECTION sec;
  int64_t addend;
  int32_t disp;
  uint8_t rtype;
  const char *name;
} x64_rip;

static void x64_rip_rel(x64_rip *rip, OPERAND k, uint64_t imm, rel r,
                        uint64_t here, int full_len) {
  rip->use = 0;
  rip->rtype = REL_PC32;
  rip->name = r.name;
  rip->disp = 0;
  if (k == OPERAND_IMM) {
    rip->disp = (int32_t)imm;
    return;
  }
  if (k != OPERAND_REL)
    return;
  if (r.section == SECTION_TEXT) {
    rip->disp = (int32_t)((int64_t)r.adrs - (int64_t)here - full_len);
    return;
  }
  rip->use = 1;
  rip->sec = r.section;
  rip->addend = r.addend - 4;
  rip->disp = 0;
}

static int x64_build(uint8_t *out, int cap, const x64_form *f, WIDTH width,
                     OPERAND oa, uint64_t ia, rel ra, OPERAND ob, uint64_t ib,
                     rel rb, uint64_t here, x64_rip *rip) {
  x64_w w = {out, 0, cap};
  uint8_t ca = (oa == OPERAND_REG) ? x64_rc(ia) : 0;
  uint8_t cb = (ob == OPERAND_REG) ? x64_rc(ib) : 0;
  int q = x64_is_q(width);
  int isb = x64_is_b(width);
  memset(rip, 0, sizeof *rip);
  switch (f->kind) {
  case X64_NP: {

    if (!x64_mode64 && f->oplen == 2 && f->op[0] == 0x0F && f->op[1] == 0x05) {
      x64_b(&w, 0xCD);
      x64_b(&w, 0x80);
      break;
    }
    if (!x64_mode64 && f->oplen >= 2 && f->op[0] == 0x48) {
      x64_n(&w, f->op + 1, f->oplen - 1);
      break;
    }
    x64_n(&w, f->op, f->oplen);
    break;
  }
  case X64_PLUSR: {
    int rexw = x64_mode64 && (f->op[0] == 0x0F) && q;
    int rexb = x64_mode64 && x64_hi(ca);
    if (rexw || rexb)
      x64_b(&w, x64_rex(rexw, 0, 0, rexb));
    if (f->oplen == 2)
      x64_b(&w, f->op[0]);
    x64_b(&w, (uint8_t)(f->op[f->oplen - 1] + (ca & 7)));
    break;
  }
  case X64_MOVIMM: {
    x64_pfx(&w, width, q, 0, x64_mode64 && x64_hi(ca), ca, 0);
    if (isb && ((int64_t)ib < -128 || (int64_t)ib > 255)) {
      masm_dief("x86", "byte immediate out of range");
    }
    if (isb) {
      x64_b(&w, (uint8_t)(0xB0 + (ca & 7)));
      x64_b(&w, (uint8_t)ib);
    } else if (x64_is_w(width)) {
      x64_b(&w, (uint8_t)(0xB8 + (ca & 7)));
      x64_b(&w, (uint8_t)ib);
      x64_b(&w, (uint8_t)(ib >> 8));
    } else if (width == WIDTH_DWORD || !x64_mode64) {
      x64_b(&w, (uint8_t)(0xB8 + (ca & 7)));
      x64_i32(&w, (uint32_t)ib);
    } else {
      x64_b(&w, (uint8_t)(0xB8 + (ca & 7)));
      x64_i64(&w, ib);
    }
    break;
  }
  case X64_RM:
  case X64_MR: {
    uint8_t reg = (f->kind == X64_RM) ? cb : ca;
    uint8_t rm = (f->kind == X64_RM) ? ca : cb;
    int rexw = q && !isb;
    x64_pfx(&w, width, rexw, x64_mode64 && x64_hi(reg),
            x64_mode64 && x64_hi(rm), ca, cb);
    uint8_t op[4];
    memcpy(op, f->op, 4);
    if (isb && f->oplen >= 1) {
      uint8_t last = op[f->oplen - 1];
      if (last == 0x85)
        op[f->oplen - 1] = 0x84;
      else if (last == 0x87)
        op[f->oplen - 1] = 0x86;
      else if ((last & 1) && last < 0x40)
        op[f->oplen - 1] = (uint8_t)(last - 1);
      else if (last == 0x89 || last == 0xC1)
        op[f->oplen - 1] = (uint8_t)(last - 1);
    }
    x64_n(&w, op, f->oplen);
    if ((f->op[0] == 0x0F && (f->op[1] == 0x40) && f->oplen == 2))
      w.p ? (w.p[w.n - 1] = (uint8_t)(0x40 + f->ext)) : 0;
    x64_b(&w, x64_modrm(3, reg, rm));
    break;
  }
  case X64_EXT: {
    int rexw = q && !isb;
    x64_pfx(&w, width, rexw, 0, x64_mode64 && x64_hi(ca), ca, 0);
    uint8_t op = f->op[0];
    if (isb && (op == 0xF7 || op == 0xFF))
      op = (op == 0xFF) ? 0xFE : 0xF6;
    x64_b(&w, op);
    x64_b(&w, x64_modrm(3, f->ext, ca));
    break;
  }
  case X64_EXTIMM: {
    int64_t imm = (int64_t)ib;
    int rexw = q && !isb;
    int bt_imm8 = (f->op[0] == 0x0F && f->op[1] == 0xBA);
    int is_test = (f->op[0] == 0xF7);
    int use83 = !isb && !is_test && !bt_imm8 && x64_imm8(imm);
    x64_pfx(&w, width, rexw, 0, x64_mode64 && x64_hi(ca), ca, 0);
    if (bt_imm8) {
      x64_b(&w, 0x0F);
      x64_b(&w, 0xBA);
      x64_b(&w, x64_modrm(3, f->ext, ca));
      x64_b(&w, (uint8_t)imm);
    } else if (is_test) {
      x64_b(&w, isb ? 0xF6 : 0xF7);
      x64_b(&w, x64_modrm(3, 0, ca));
      if (isb)
        x64_b(&w, (uint8_t)imm);
      else if (x64_is_w(width)) {
        x64_b(&w, (uint8_t)imm);
        x64_b(&w, (uint8_t)(imm >> 8));
      } else
        x64_i32(&w, (uint32_t)imm);
    } else {
      uint8_t op = isb ? 0x80 : (use83 ? 0x83 : 0x81);
      x64_b(&w, op);
      x64_b(&w, x64_modrm(3, f->ext, ca));
      if (isb || use83)
        x64_b(&w, (uint8_t)imm);
      else if (x64_is_w(width)) {
        x64_b(&w, (uint8_t)imm);
        x64_b(&w, (uint8_t)(imm >> 8));
      } else
        x64_i32(&w, (uint32_t)imm);
    }
    break;
  }
  case X64_SHIFT: {
    int by_cl = (ob == OPERAND_REG);
    if (!by_cl && ((int64_t)ib < 0 || (int64_t)ib > 255)) {
      masm_dief("x86", "shift count out of range");
    }
    if (by_cl && ib != 4) {
      fprintf(stderr, "masm: x86: shift count register must be r4 (cl)\n");
      return -1;
    }
    int rexw = q && !isb;
    x64_pfx(&w, width, rexw, 0, x64_mode64 && x64_hi(ca), ca, 0);
    uint8_t op;
    if (by_cl)
      op = isb ? 0xD2 : 0xD3;
    else if (!isb && ib == 1)
      op = 0xD1;
    else if (isb && ib == 1)
      op = 0xD0;
    else
      op = isb ? 0xC0 : 0xC1;
    x64_b(&w, op);
    x64_b(&w, x64_modrm(3, f->ext, ca));
    if (!by_cl && ib != 1)
      x64_b(&w, (uint8_t)ib);
    break;
  }
  case X64_REL32: {
    int32_t rel32 = 0;
    if (oa == OPERAND_IMM)
      rel32 = (int32_t)ia;
    else if (oa == OPERAND_REL && ra.section == SECTION_TEXT)
      rel32 = (int32_t)((int64_t)ra.adrs - (int64_t)here - 5);
    else if (oa == OPERAND_REL) {
      rip->use = 1;
      rip->at = 1;
      rip->sec = ra.section;
      rip->addend = (int64_t)ra.adrs - 4;
      rip->rtype = REL_PC32;
      rip->name = ra.name;
    }
    x64_b(&w, f->op[0]);
    x64_i32(&w, (uint32_t)rel32);
    break;
  }
  case X64_JCC: {
    int32_t rel32 = 0;
    if (oa == OPERAND_IMM)
      rel32 = (int32_t)ia;
    else if (oa == OPERAND_REL && ra.section == SECTION_TEXT)
      rel32 = (int32_t)((int64_t)ra.adrs - (int64_t)here - 6);
    else if (oa == OPERAND_REL) {
      rip->use = 1;
      rip->at = 2;
      rip->sec = ra.section;
      rip->addend = (int64_t)ra.adrs - 4;
      rip->rtype = REL_PC32;
      rip->name = ra.name;
    }
    x64_b(&w, 0x0F);
    x64_b(&w, (uint8_t)(0x80 + f->ext));
    x64_i32(&w, (uint32_t)rel32);
    break;
  }
  case X64_SETCC: {
    int rexb = x64_mode64 && (x64_hi(ca) || ca >= 4);
    if (rexb)
      x64_b(&w, x64_rex(0, 0, 0, x64_hi(ca)));
    x64_b(&w, 0x0F);
    x64_b(&w, (uint8_t)(0x90 + f->ext));
    x64_b(&w, x64_modrm(3, 0, ca));
    break;
  }
  case X64_LEA: {
    if (x64_mode64) {
      x64_rip_rel(rip, ob, ib, rb, here, 7);
      if (rip->use)
        rip->at = 3;
      x64_pfx(&w, WIDTH_QWORD, 1, x64_hi(ca), 0, ca, 0);
      x64_b(&w, 0x8D);
      x64_b(&w, x64_modrm(0, ca, 5));
      x64_i32(&w, (uint32_t)rip->disp);
    } else {

      rip->use = 1;
      rip->at = x64_is_w(width) ? 2 : 1;
      rip->sec = rb.section;
      rip->addend = (int64_t)rb.adrs;
      rip->rtype = REL_ABS32;
      rip->name = rb.name;
      if (x64_is_w(width)) {
        x64_b(&w, 0x66);
        x64_b(&w, (uint8_t)(0xB8 + (ca & 7)));
        x64_b(&w, 0);
        x64_b(&w, 0);
      } else {
        x64_b(&w, (uint8_t)(0xB8 + (ca & 7)));
        x64_i32(&w, 0);
      }
    }
    break;
  }
  case X64_LOAD:
  case X64_STORE: {
    if (x64_mode64) {
      int rexw = q && !isb;
      int rfield = (f->kind == X64_LOAD) ? x64_hi(ca) : x64_hi(ca);
      int pfx = (x64_is_w(width) ? 1 : 0) +
                ((rexw || rfield || x64_rex8(width, ca)) ? 1 : 0);
      int full = pfx + 1 + 1 + 4;
      x64_rip_rel(rip, ob, ib, rb, here, full);
      if (rip->use)
        rip->at = pfx + 2;
      x64_pfx(&w, width, rexw, x64_hi(ca), 0, ca, 0);
      uint8_t op = isb ? (f->kind == X64_LOAD ? 0x8A : 0x88)
                       : (f->kind == X64_LOAD ? 0x8B : 0x89);
      x64_b(&w, op);
      x64_b(&w, x64_modrm(0, ca, 5));
      x64_i32(&w, (uint32_t)rip->disp);
    } else {

      int isw = x64_is_w(width);
      rip->use = 1;
      rip->at = (isw ? 1 : 0) + 2;
      rip->sec = rb.section;
      rip->addend = (int64_t)rb.adrs;
      rip->rtype = REL_ABS32;
      rip->name = rb.name;
      if (isw)
        x64_b(&w, 0x66);
      x64_b(&w, isb ? (f->kind == X64_LOAD ? 0x8A : 0x88)
                    : (f->kind == X64_LOAD ? 0x8B : 0x89));
      x64_b(&w, x64_modrm(0, ca, 5));
      x64_i32(&w, 0);
    }
    break;
  }
  case X64_INT:
    x64_b(&w, 0xCD);
    x64_b(&w, (uint8_t)ia);
    break;
  case X64_IMUL3: {
    int64_t imm = (int64_t)ib;
    int rexw = q;
    int use_ib = x64_imm8(imm);
    x64_pfx(&w, width, rexw, x64_mode64 && x64_hi(ca),
            x64_mode64 && x64_hi(ca), ca, ca);
    x64_b(&w, use_ib ? 0x6B : 0x69);
    x64_b(&w, x64_modrm(3, ca, ca));
    if (use_ib)
      x64_b(&w, (uint8_t)imm);
    else if (x64_is_w(width)) {
      x64_b(&w, (uint8_t)imm);
      x64_b(&w, (uint8_t)(imm >> 8));
    } else
      x64_i32(&w, (uint32_t)imm);
    break;
  }
  default:
    return -1;
  }
  return w.n;
}

static int x64_is_cmov(const x64_form *f) {
  return f->kind == X64_MR && f->oplen == 2 && f->op[0] == 0x0F &&
         f->op[1] == 0x40;
}

static int x64_build_fix(uint8_t *out, int cap, const x64_form *f, WIDTH width,
                         OPERAND oa, uint64_t ia, rel ra, OPERAND ob,
                         uint64_t ib, rel rb, uint64_t here, x64_rip *rip) {
  int n = x64_build(out, cap, f, width, oa, ia, ra, ob, ib, rb, here, rip);
  if (n > 0 && out && x64_is_cmov(f)) {
    for (int i = n - 2; i >= 0; i--) {
      if (out[i] == 0x40) {
        out[i] = (uint8_t)(0x40 + f->ext);
        break;
      }
    }
  }
  if (n > 0 && out && f->kind == X64_MR && f->oplen == 2 && f->op[0] == 0x0F &&
      (f->op[1] == 0xB6 || f->op[1] == 0xBE) && width == WIDTH_WORD) {
    for (int i = 0; i < n; i++) {
      if (out[i] == 0xB6) {
        out[i] = 0xB7;
        break;
      }
      if (out[i] == 0xBE) {
        out[i] = 0xBF;
        break;
      }
    }
  }
  return n;
}

static void x86_encode(context *ctx, char *mnemonic, WIDTH width, OPERAND oa,
                       uint64_t ia, rel ra, OPERAND ob, uint64_t ib, rel rb) {
  if (mnemonic[0] == '.')
    return;
  x64_mode64 = !(ctx && ctx->arch && strcmp(ctx->arch, "x86") == 0);
  if (!x64_mode64 && strcmp(mnemonic, "movsxd") == 0) {
    fprintf(stderr, "masm: x86: movsxd requires x86_64\n");
    exit(1);
  }
  const x64_form *f = x64_find(mnemonic, oa, ob, width);
  if (!f) {
    masm_dief(x64_mode64 ? "x86_64" : "x86", "no form for '%s'", mnemonic);
  }
  uint8_t buf[32];
  x64_rip rip;
  int n = x64_build_fix(buf, 32, f, width, oa, ia, ra, ob, ib, rb,
                        *ctx->current_text_adrs, &rip);
  if (n <= 0 || n > 32) {
    fprintf(stderr, "masm: x86%s: encode failed for '%s'\n",
            x64_mode64 ? "_64" : "", mnemonic);
    exit(1);
  }
  if (ctx->text_stream) {
    if (rip.use) {
      rel *rr = ctx_new_rel(ctx);
      rr->adrs = *ctx->current_text_adrs + (uint64_t)rip.at;
      rr->section = rip.sec;
      rr->addend = rip.addend;
      rr->type = rip.rtype;
      rr->name = rip.name;
    }
    if (fwrite(buf, 1, (size_t)n, ctx->text_stream) != (size_t)n) {
      fprintf(stderr, "masm: x86%s: text stream full\n",
              x64_mode64 ? "_64" : "");
      exit(1);
    }
  }
  *ctx->current_text_adrs += (uint64_t)n;
}

static registry_object x86_64_object = {
    REGISTRY_ENCODER, "x86_64", NULL, x86_encode, NULL,
};
static registry_object x86_32_object = {
    REGISTRY_ENCODER, "x86", NULL, x86_encode, NULL,
};

static void x86_register(void) __attribute__((constructor));
static void x86_register(void) {
  register_object(&x86_64_object);
  register_object(&x86_32_object);
}

#endif
