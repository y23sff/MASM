#ifndef MASM_X26A_H
#define MASM_X26A_H

#include "X26.h"
#include "../core.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XA_MAX_SRC   (1u << 22)
#define XA_MAX_LINE  512
#define XA_MAX_DEFS  256
#define XA_MAX_INC   64
#define XA_MAX_NAME  64
#define XA_MAX_PATH  512
#define XA_MAX_PEND  128
#define XA_MAX_IF    32
#define XA_MAX_SYMS  256

static char xa_src[XA_MAX_SRC];
static size_t xa_len;
static char xa_defs[XA_MAX_DEFS][XA_MAX_NAME];
static int xa_ndefs;
static char xa_inc[XA_MAX_INC][XA_MAX_PATH];
static int xa_ninc;
static char xa_cur[XA_MAX_PATH];
static int xa_ifs[XA_MAX_IF];
static int xa_nif;
static char xa_pend[XA_MAX_PEND][XA_MAX_NAME];
static int xa_npend;

static void xa_die(const char *msg) {
  fprintf(stderr, "masm: x26a: %s\n", msg);
  exit(1);
}

static void xa_die2(const char *msg, const char *what) {
  fprintf(stderr, "masm: x26a: %s '%s'\n", msg, what);
  exit(1);
}

static void xa_add(const char *s, size_t n) {
  if (xa_len + n + 1 >= XA_MAX_SRC)
    xa_die("source too large");
  memcpy(xa_src + xa_len, s, n);
  xa_len += n;
}

static char *xa_trim(char *s) {
  while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
    s++;
  size_t n = strlen(s);
  while (n && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r' ||
               s[n - 1] == '\n'))
    s[--n] = 0;
  return s;
}

static int xa_has_def(const char *n) {
  for (int i = 0; i < xa_ndefs; i++)
    if (strcmp(xa_defs[i], n) == 0)
      return 1;
  return 0;
}

static int xa_taking(void) {
  for (int i = 0; i < xa_nif; i++)
    if (!xa_ifs[i])
      return 0;
  return 1;
}

static int xa_seen(const char *p) {
  for (int i = 0; i < xa_ninc; i++)
    if (strcmp(xa_inc[i], p) == 0)
      return 1;
  return 0;
}

static int xa_is_extern(context *ctx, const char *n) {
  for (int i = 0; i < ctx->nsyms; i++)
    if (ctx->syms[i].kind == SYM_EXTERN && strcmp(ctx->syms[i].name, n) == 0)
      return 1;
  return 0;
}

static void xa_extern(context *ctx, const char *n) {
  if (xa_is_extern(ctx, n))
    return;
  sym *s = ctx_new_sym(ctx);
  snprintf(s->name, XA_MAX_NAME, "%s", n);
  s->kind = SYM_EXTERN;
  s->section = SECTION_NULL;
  s->off = 0;
}

static void xa_slurp(const char *path);

static void xa_directive(char *d, context *ctx) {
  char *sp = d;
  while (*sp && *sp != ' ' && *sp != '\t')
    sp++;
  char saved = *sp;
  *sp = 0;
  char *rest = saved ? xa_trim(sp + 1) : (char *)"";
  const char *kw = d;

  if (strcmp(kw, "endif") == 0) {
    if (!xa_nif)
      xa_die("endif without ifdef");
    xa_nif--;
    return;
  }
  if (strcmp(kw, "else") == 0) {
    if (!xa_nif)
      xa_die("else without ifdef");
    int parent = 1;
    for (int i = 0; i < xa_nif - 1; i++)
      if (!xa_ifs[i])
        parent = 0;
    xa_ifs[xa_nif - 1] = parent && !xa_ifs[xa_nif - 1];
    return;
  }
  if (strcmp(kw, "ifdef") == 0 || strcmp(kw, "ifndef") == 0) {
    if (!rest[0])
      xa_die2(kw, "needs a name");
    if (xa_nif >= XA_MAX_IF)
      xa_die("ifdef nesting too deep");
    int has = xa_has_def(rest);
    if (strcmp(kw, "ifndef") == 0)
      has = !has;
    xa_ifs[xa_nif++] = xa_taking() && has;
    return;
  }
  if (strcmp(kw, "define") != 0 && strcmp(kw, "include") != 0 &&
      strcmp(kw, "export") != 0 && strcmp(kw, "extern") != 0)
    xa_die2("unknown directive", kw);
  if (!xa_taking())
    return;
  if (strcmp(kw, "define") == 0) {
    if (!rest[0] || !def_valid_lab(rest))
      xa_die2("bad define name", rest);
    if (xa_has_def(rest))
      return;
    if (xa_ndefs >= XA_MAX_DEFS)
      xa_die("too many defines");
    snprintf(xa_defs[xa_ndefs++], XA_MAX_NAME, "%s", rest);
    return;
  }
  if (strcmp(kw, "include") == 0) {
    if (rest[0] != '"')
      xa_die("include needs \"file\"");
    char *e = strchr(rest + 1, '"');
    if (!e)
      xa_die("include needs \"file\"");
    *e = 0;
    xa_slurp(rest + 1);
    return;
  }
  if (strcmp(kw, "export") == 0) {
    if (!rest[0] || !def_valid_lab(rest))
      xa_die2("bad export name", rest);
    if (xa_npend >= XA_MAX_PEND)
      xa_die("too many exports");
    snprintf(xa_pend[xa_npend++], XA_MAX_NAME, "%s", rest);
    return;
  }
  if (strcmp(kw, "extern") == 0) {
    if (!rest[0] || !def_valid_lab(rest))
      xa_die2("bad extern name", rest);
    xa_extern(ctx, rest);
    return;
  }
  xa_die2("unknown directive", kw);
}

static void xa_resolve(const char *in, char *out, size_t cap) {
  char buf[XA_MAX_PATH];
  if (in[0] == '/') {
    snprintf(buf, sizeof buf, "%s", in);
  } else {
    const char *slash = strrchr(xa_cur, '/');
    if (slash)
      snprintf(buf, sizeof buf, "%.*s/%s", (int)(slash - xa_cur), xa_cur, in);
    else
      snprintf(buf, sizeof buf, "%s", in);
  }
  char rp[XA_MAX_PATH];
  if (realpath(buf, rp))
    snprintf(out, cap, "%s", rp);
  else
    snprintf(out, cap, "%s", buf);
}

static void xa_slurp(const char *path) {
  char resolved[XA_MAX_PATH];
  xa_resolve(path, resolved, sizeof resolved);
  if (xa_seen(resolved))
    return;
  if (xa_ninc >= XA_MAX_INC)
    xa_die("too many includes");
  snprintf(xa_inc[xa_ninc++], XA_MAX_PATH, "%s", resolved);
  FILE *f = fopen(resolved, "r");
  if (!f) {
    fprintf(stderr, "masm: x26a: cannot open %s\n", resolved);
    exit(1);
  }
  char prev[XA_MAX_PATH];
  snprintf(prev, sizeof prev, "%s", xa_cur);
  snprintf(xa_cur, sizeof xa_cur, "%s", resolved);
  char buf[XA_MAX_LINE];
  while (fgets(buf, sizeof buf, f)) {
    char *s = buf;
    for (char *q = s; *q; q++)
      if (*q == ';') {
        *q = 0;
        break;
      }
    s = xa_trim(s);
    if (!*s)
      continue;
    if (*s == '#') {
      xa_directive(xa_trim(s + 1), def_ctx);
      continue;
    }
    if (strncmp(s, ".globl", 6) == 0 || strncmp(s, ".global", 7) == 0 ||
        strncmp(s, "globl", 5) == 0 || strncmp(s, "global", 6) == 0)
      xa_die("use #export instead of .globl");
    if (xa_taking()) {
      xa_add(s, strlen(s));
      xa_add("\n", 1);
    }
  }
  fclose(f);
  snprintf(xa_cur, sizeof xa_cur, "%s", prev);
}

static void xa_builtin(const char *prefix, const char *name) {
  char b[XA_MAX_NAME];
  size_t n = 0;
  while (name[n] && n + 8 < sizeof b) {
    b[n] = (char)toupper((unsigned char)name[n]);
    n++;
  }
  b[n] = 0;
  char full[XA_MAX_NAME];
  snprintf(full, sizeof full, "%s_%s", prefix, b);
  if (xa_has_def(full))
    return;
  if (xa_ndefs >= XA_MAX_DEFS)
    xa_die("too many defines");
  snprintf(xa_defs[xa_ndefs++], XA_MAX_NAME, "%s", full);
}

#define XA_NOP_MIN 16

static void xa_word(const char *s, char *w, size_t cap) {
  size_t n = 0;
  while (s[n] && s[n] != ' ' && s[n] != '\t' && s[n] != ':')
    n++;
  if (n >= cap)
    n = cap - 1;
  memcpy(w, s, n);
  w[n] = 0;
}

static int xa_is_label_line(const char *s) {
  size_t n = 0;
  if (!(isalpha((unsigned char)s[0]) || s[0] == '_' || s[0] == '.' ||
        s[0] == '$'))
    return 0;
  while (isalnum((unsigned char)s[n]) || s[n] == '_' || s[n] == '.' ||
         s[n] == '$')
    n++;
  while (s[n] == ' ' || s[n] == '\t')
    n++;
  return s[n] == ':';
}

static int xa_terminal(const char *w) {
  return strcmp(w, "jmp") == 0 || strcmp(w, "b") == 0 ||
         strcmp(w, "ret") == 0;
}

static size_t xa_addline(char *dst, size_t at, size_t cap, const char *l) {
  size_t n = strlen(l);
  if (at + n + 1 >= cap) {
    fprintf(stderr, "masm: x26a: optimizer buffer full\n");
    exit(1);
  }
  memcpy(dst + at, l, n);
  memcpy(dst + at + n, "\n", 1);
  return at + n + 1;
}

static size_t xa_emit_noploop(context *ctx, char *dst, size_t at, size_t cap,
                              long n, int id) {
  char l[96];
  const char *arch = ctx->arch ? ctx->arch : "x86_64";
  at = xa_addline(dst, at, cap, "push tmp");
  snprintf(l, sizeof l, "mov tmp, %ld", n);
  at = xa_addline(dst, at, cap, l);
  snprintf(l, sizeof l, "__masm_nop%d:", id);
  at = xa_addline(dst, at, cap, l);
  at = xa_addline(dst, at, cap, "nop");
  at = xa_addline(dst, at, cap, "dec tmp");
  at = xa_addline(dst, at, cap, "test tmp, tmp");
  snprintf(l, sizeof l, "jnz __masm_nop%d", id);
  at = xa_addline(dst, at, cap, l);
  at = xa_addline(dst, at, cap, "pop tmp");
  return at;
}

static void xa_optimize(context *ctx) {
  if (!ctx->optimize)
    return;
  const char *arch = ctx->arch ? ctx->arch : "x86_64";
  int no_loop = strcmp(arch, "wasm32") == 0 || strcmp(arch, "wasm64") == 0;
  static char out[XA_MAX_SRC];
  size_t ro = 0, wo = 0;
  int dead = 0;
  int loops = 0;
  char w[64];
  while (ro < xa_len) {
    size_t e = ro;
    while (e < xa_len && xa_src[e] != '\n')
      e++;
    xa_src[e] = 0;
    char *line = xa_src + ro;
    size_t next = e + 1;
    if (dead) {
      if (xa_is_label_line(line) || line[0] == '.') {
        dead = 0;
      } else {
        ro = next;
        if (next > xa_len)
          break;
        continue;
      }
    }
    if (line[0] == '.') {
      wo = xa_addline(out, wo, sizeof out, line);
      ro = next;
      if (next > xa_len)
        break;
      continue;
    }
    xa_word(line, w, sizeof w);
    if (!xa_is_label_line(line) && strcmp(w, "nop") == 0 && !no_loop) {
      size_t rr = next;
      long run = 1;
      while (rr < xa_len) {
        size_t ee = rr;
        while (ee < xa_len && xa_src[ee] != '\n')
          ee++;
        if (ee < xa_len)
          xa_src[ee] = 0;
        if (strcmp(xa_src + rr, "nop") != 0) {
          if (ee < xa_len)
            xa_src[ee] = '\n';
          break;
        }
        run++;
        rr = ee + 1;
      }
      if (run >= XA_NOP_MIN) {
        wo = xa_emit_noploop(ctx, out, wo, sizeof out, run, loops++);
        ro = rr;
        continue;
      }
    }
    wo = xa_addline(out, wo, sizeof out, line);
    if (xa_terminal(w) && !xa_is_label_line(line))
      dead = 1;
    ro = next;
    if (next > xa_len)
      break;
  }
  memcpy(xa_src, out, wo);
  xa_len = wo;
  xa_src[wo] = 0;
  if (getenv("MASM_DUMP_OPT")) {
    xa_src[wo] = 0;
    fprintf(stderr, "---OPT SOURCE---\n");
    fprintf(stderr, "%s\n", xa_src);
    fprintf(stderr, "---END---\n");
  }
}

static void xa_parse(context *ctx, char *input_file) {
  registry_object *encobj =
      find_registry((char *)(ctx->arch ? ctx->arch : "x86_64"));
  if (!encobj || encobj->kind != REGISTRY_ENCODER || !encobj->encoder) {
    fprintf(stderr, "masm: x26a: no encoder registered for arch '%s'\n",
            ctx->arch ? ctx->arch : "?");
    exit(1);
  }
  encode enc = encobj->encoder;

  def_ctx = ctx;
  def_src_len = 0;
  def_ninc = 0;
  def_nlabs = 0;
  def_nins = 0;
  def_lineno = 0;
  def_sec = SECTION_TEXT;
  *ctx->current_text_adrs = 0;
  ctx->input_name = input_file;
  xa_len = 0;
  xa_ninc = 0;
  xa_ndefs = 0;
  xa_nif = 0;
  xa_npend = 0;
  xa_cur[0] = 0;
  xa_builtin("ARCH", ctx->arch ? ctx->arch : "x86_64");
  xa_builtin("FORMAT", ctx->format ? ctx->format : "elf64");
  xa_slurp(input_file);
  if (xa_nif)
    xa_die("missing endif");
  xa_src[xa_len] = 0;
  xa_optimize(ctx);

  const char *p = xa_src;
  while (*p) {
    const char *e = p;
    while (*e && *e != '\n')
      e++;
    size_t n = (size_t)(e - p);
    if (n >= DEF_MAX_LINE)
      def_die("line too long");
    memcpy(def_line, p, n);
    def_line[n] = 0;
    def_lineno++;
    def_one_line(enc);
    p = *e ? e + 1 : e;
  }

  for (int i = 0; i < xa_npend; i++) {
    int li = def_find_lab(xa_pend[i]);
    if (li < 0 || !def_labs[li].defined)
      xa_die2("export of undefined label", xa_pend[i]);
    int found = -1;
    for (int k = 0; k < ctx->nsyms; k++)
      if (strcmp(ctx->syms[k].name, xa_pend[i]) == 0) {
        found = k;
        break;
      }
    sym *s;
    if (found < 0) {
      s = ctx_new_sym(ctx);
      snprintf(s->name, XA_MAX_NAME, "%s", xa_pend[i]);
    } else {
      s = &ctx->syms[found];
    }
    s->kind = SYM_EXPORT;
    s->section = def_labs[li].section;
    s->off = def_labs[li].off;
  }

  for (int i = 0; i < def_nins; i++) {
    def_instr *in = &def_ins[i];
    if (in->mn[0] == '.')
      continue;
    if (in->oa == OPERAND_REL && in->name_a[0] &&
        def_find_lab(in->name_a) < 0 && !xa_is_extern(ctx, in->name_a)) {
      def_lineno = in->line;
      fprintf(stderr, "masm: line %u: undefined label '%s'\n", def_lineno,
              in->name_a);
      exit(1);
    }
    if (in->ob == OPERAND_REL && in->name_b[0] &&
        def_find_lab(in->name_b) < 0 && !xa_is_extern(ctx, in->name_b)) {
      def_lineno = in->line;
      fprintf(stderr, "masm: line %u: undefined label '%s'\n", def_lineno,
              in->name_b);
      exit(1);
    }
  }

  *ctx->current_text_adrs = 0;
  fseek(ctx->text_stream, 0, SEEK_SET);
  for (int i = 0; i < def_nins; i++) {
    def_lineno = def_ins[i].line;
    def_resolve(&def_ins[i], 0);
    def_resolve(&def_ins[i], 1);
    enc(ctx, def_ins[i].mn, def_ins[i].width, def_ins[i].oa, def_ins[i].ia,
        def_ins[i].ra, def_ins[i].ob, def_ins[i].ib, def_ins[i].rb);
  }
}

static registry_object x26a_object = {
    REGISTRY_PARSER, "X26A", xa_parse, NULL, NULL,
};

static void x26a_register(void) __attribute__((constructor));
static void x26a_register(void) { register_object(&x26a_object); }

#endif
