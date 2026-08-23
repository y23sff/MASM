#ifndef MASM_DEFAULT_H
#define MASM_DEFAULT_H

#include "../core.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEF_MAX_SRC   (1u << 22)
#define DEF_MAX_LINE  512
#define DEF_MAX_INSTR 8192
#define DEF_MAX_LABEL 1024
#define DEF_MAX_NAME  64
#define DEF_MAX_INC   32
#define DEF_MAX_PATH  512

typedef struct {
  char name[DEF_MAX_NAME];
  SECTION section;
  uint64_t off;
  int defined;
} def_label;

typedef struct {
  char mn[DEF_MAX_NAME + 8];
  WIDTH width;
  OPERAND oa, ob;
  uint64_t ia, ib;
  rel ra, rb;
  char name_a[DEF_MAX_NAME];
  char name_b[DEF_MAX_NAME];
  uint64_t off;
  uint32_t line;
} def_instr;

static char def_src[DEF_MAX_SRC];
static size_t def_src_len;
static char def_inc[DEF_MAX_INC][DEF_MAX_PATH];
static int def_ninc;
static char def_cur[DEF_MAX_PATH];
static char def_line[DEF_MAX_LINE];
static uint32_t def_lineno;
static SECTION def_sec;
static def_label *def_labs;
static int def_nlabs, def_alabs;
static def_instr *def_ins;
static int def_nins, def_ains;

static void def_need_labs(int n) {
  if (n < def_alabs)
    return;
  int nc = def_alabs ? def_alabs * 2 : 1024;
  while (nc < n)
    nc *= 2;
  def_labs = (def_label *)realloc(def_labs, (size_t)nc * sizeof(def_label));
  if (!def_labs) {
    fprintf(stderr, "masm: out of memory\n");
    exit(1);
  }
  def_alabs = nc;
}

static void def_need_ins(int n) {
  if (n < def_ains)
    return;
  int nc = def_ains ? def_ains * 2 : 8192;
  while (nc < n)
    nc *= 2;
  def_ins = (def_instr *)realloc(def_ins, (size_t)nc * sizeof(def_instr));
  if (!def_ins) {
    fprintf(stderr, "masm: out of memory\n");
    exit(1);
  }
  def_ains = nc;
}
static context *def_ctx;

static void def_die(const char *msg) {
  fprintf(stderr, "masm: line %u: %s\n", def_lineno, msg);
  exit(1);
}

static void def_die2(const char *msg, const char *what) {
  fprintf(stderr, "masm: line %u: %s '%s'\n", def_lineno, msg, what);
  exit(1);
}

static char *def_trim(char *s) {
  while (*s == ' ' || *s == '\t' || *s == '\r')
    s++;
  size_t n = strlen(s);
  while (n && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r'))
    s[--n] = 0;
  return s;
}

static void def_strip_comment(char *s) {
  int in_str = 0, esc = 0;
  for (char *p = s; *p; p++) {
    if (in_str) {
      if (esc) {
        esc = 0;
        continue;
      }
      if (*p == '\\') {
        esc = 1;
        continue;
      }
      if (*p == '"')
        in_str = 0;
      continue;
    }
    if (*p == '"') {
      in_str = 1;
      continue;
    }
    if (*p == ';' || *p == '#') {
      *p = 0;
      return;
    }
  }
}

static int def_already(const char *path) {
  for (int i = 0; i < def_ninc; i++)
    if (strcmp(def_inc[i], path) == 0)
      return 1;
  return 0;
}

static void def_src_add(const char *s, size_t n) {
  if (def_src_len + n + 1 >= DEF_MAX_SRC)
    def_die("source too large");
  memcpy(def_src + def_src_len, s, n);
  def_src_len += n;
}

static int def_is_include(const char *line, char *path) {
  const char *p = line;
  while (*p == ' ' || *p == '\t')
    p++;
  if (strncmp(p, "#include", 8) != 0)
    return 0;
  p += 8;
  while (*p == ' ' || *p == '\t')
    p++;
  if (*p != '"')
    return 0;
  p++;
  const char *e = strchr(p, '"');
  if (!e)
    return 0;
  size_t n = (size_t)(e - p);
  if (!n || n >= DEF_MAX_NAME)
    return 0;
  memcpy(path, p, n);
  path[n] = 0;
  return 1;
}

static void def_resolve_path(const char *in, char *out, size_t cap) {
  char buf[DEF_MAX_PATH];
  if (in[0] == '/') {
    snprintf(buf, sizeof buf, "%s", in);
  } else {
    const char *slash = strrchr(def_cur, '/');
    if (slash)
      snprintf(buf, sizeof buf, "%.*s/%s", (int)(slash - def_cur), def_cur,
               in);
    else
      snprintf(buf, sizeof buf, "%s", in);
  }
  char rp[DEF_MAX_PATH];
  if (realpath(buf, rp))
    snprintf(out, cap, "%s", rp);
  else
    snprintf(out, cap, "%s", buf);
}

static void def_slurp(const char *path) {
  char resolved[DEF_MAX_PATH];
  def_resolve_path(path, resolved, sizeof resolved);
  if (def_already(resolved))
    return;
  if (def_ninc >= DEF_MAX_INC)
    def_die("too many includes");
  snprintf(def_inc[def_ninc++], DEF_MAX_PATH, "%s", resolved);
  FILE *f = fopen(resolved, "r");
  if (!f) {
    fprintf(stderr, "masm: cannot open %s\n", resolved);
    exit(1);
  }
  char prev[DEF_MAX_PATH];
  snprintf(prev, sizeof prev, "%s", def_cur);
  snprintf(def_cur, sizeof def_cur, "%s", resolved);
  char buf[DEF_MAX_LINE], inc[DEF_MAX_NAME];
  while (fgets(buf, sizeof buf, f)) {
    if (def_is_include(buf, inc))
      def_slurp(inc);
    else
      def_src_add(buf, strlen(buf));
  }
  fclose(f);
  snprintf(def_cur, sizeof def_cur, "%s", prev);
}

static FILE *def_secfile(void) {
  switch (def_sec) {
  case SECTION_TEXT:
    return def_ctx->text_stream;
  case SECTION_RODATA:
    return def_ctx->rodata_stream;
  case SECTION_DATA:
    return def_ctx->data_stream;
  default:
    return NULL;
  }
}

static uint64_t def_secoff(void) {
  switch (def_sec) {
  case SECTION_TEXT:
    return *def_ctx->current_text_adrs;
  case SECTION_RODATA:
    return (uint64_t)ftell(def_ctx->rodata_stream);
  case SECTION_DATA:
    return (uint64_t)ftell(def_ctx->data_stream);
  case SECTION_BSS:
    return *def_ctx->bss_size;
  default:
    return 0;
  }
}

static int def_find_lab(const char *name) {
  for (int i = 0; i < def_nlabs; i++)
    if (strcmp(def_labs[i].name, name) == 0)
      return i;
  return -1;
}

static void def_def_lab(const char *name) {
  int i = def_find_lab(name);
  if (i >= 0 && def_labs[i].defined)
    def_die("redefined label");
  if (i < 0) {
    def_need_labs(def_nlabs + 1);
    i = def_nlabs++;
    snprintf(def_labs[i].name, DEF_MAX_NAME, "%s", name);
  }
  def_labs[i].section = def_sec;
  def_labs[i].off = def_secoff();
  def_labs[i].defined = 1;
  if (strcmp(name, "_start") == 0 && def_ctx) {
    def_ctx->has_start = 1;
    def_ctx->start_off = def_labs[i].off;
  }
}

static int def_valid_lab(const char *s) {
  if (!s || !*s)
    return 0;
  if (!(isalpha((unsigned char)*s) || *s == '_' || *s == '.'))
    return 0;
  for (const char *p = s + 1; *p; p++)
    if (!(isalnum((unsigned char)*p) || *p == '_' || *p == '.' || *p == '$'))
      return 0;
  return 1;
}

static char *def_split_lab(char *s, char *out) {
  int in_str = 0, esc = 0;
  for (char *p = s; *p; p++) {
    if (in_str) {
      if (esc) {
        esc = 0;
        continue;
      }
      if (*p == '\\') {
        esc = 1;
        continue;
      }
      if (*p == '"')
        in_str = 0;
      continue;
    }
    if (*p == '"') {
      in_str = 1;
      continue;
    }
    if (*p == ':') {
      size_t n = (size_t)(p - s);
      while (n && (s[n - 1] == ' ' || s[n - 1] == '\t'))
        n--;
      if (!n || n >= DEF_MAX_NAME)
        return s;
      memcpy(out, s, n);
      out[n] = 0;
      if (!def_valid_lab(out))
        return s;
      return def_trim(p + 1);
    }
  }
  return s;
}

static int def_split_ops(char *s, char out[][DEF_MAX_LINE], int max) {
  int n = 0, in_str = 0, esc = 0;
  char *start = s;
  for (char *p = s;; p++) {
    if (*p && in_str) {
      if (esc) {
        esc = 0;
        continue;
      }
      if (*p == '\\') {
        esc = 1;
        continue;
      }
      if (*p == '"')
        in_str = 0;
      continue;
    }
    if (*p == '"') {
      in_str = 1;
      continue;
    }
    if (*p == ',' || *p == 0) {
      if (n >= max)
        break;
      size_t k = (size_t)(p - start);
      while (k && (start[k - 1] == ' ' || start[k - 1] == '\t'))
        k--;
      char *t = start;
      while (k && (*t == ' ' || *t == '\t')) {
        t++;
        k--;
      }
      memcpy(out[n], t, k);
      out[n][k] = 0;
      if (out[n][0])
        n++;
      if (!*p)
        break;
      start = p + 1;
    }
  }
  return n;
}

static int def_parse_str(const char *s, char *out, size_t cap) {
  while (*s == ' ' || *s == '\t')
    s++;
  if (*s != '"')
    def_die("invalid string literal");
  s++;
  size_t n = 0;
  while (*s) {
    if (*s == '\\' && s[1]) {
      char c = s[1], w = c;
      if (c == 'n')
        w = '\n';
      else if (c == 't')
        w = '\t';
      else if (c == 'r')
        w = '\r';
      else if (c == '0')
        w = 0;
      if (n + 1 >= cap)
        def_die("string too long");
      out[n++] = w;
      s += 2;
      continue;
    }
    if (*s == '"') {
      out[n] = 0;
      return (int)n;
    }
    if (n + 1 >= cap)
      def_die("string too long");
    out[n++] = *s++;
  }
  def_die("unterminated string literal");
  return 0;
}

static int def_is_reg(const char *s, uint64_t *idx) {
  if (strcmp(s, "tmp") == 0) {
    *idx = 8;
    return 1;
  }
  if (strcmp(s, "sp") == 0) {
    *idx = 9;
    return 1;
  }
  if (strcmp(s, "fp") == 0) {
    *idx = 10;
    return 1;
  }
  if (s[0] != 'r' || !s[1])
    return 0;
  char *end = NULL;
  long v = strtol(s + 1, &end, 10);
  if (end == s + 1 || *end)
    return 0;
  if (v < 0 || v > 7)
    def_die("register out of range (r0..r7, tmp, sp, fp)");
  *idx = (uint64_t)v;
  return 1;
}

static int def_is_imm(const char *s) {
  if (!*s)
    return 0;
  if (*s == '-' || *s == '+')
    s++;
  if (!*s)
    return 0;
  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    s += 2;
    if (!*s)
      return 0;
    for (; *s; s++)
      if (!isxdigit((unsigned char)*s))
        return 0;
    return 1;
  }
  for (; *s; s++)
    if (!isdigit((unsigned char)*s))
      return 0;
  return 1;
}

static int64_t def_imm(const char *s) {
  int neg = 0;
  if (*s == '-') {
    neg = 1;
    s++;
  } else if (*s == '+')
    s++;
  errno = 0;
  int64_t v = (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
                  ? (int64_t)strtoll(s, NULL, 16)
                  : (int64_t)strtoll(s, NULL, 10);
  if (errno == ERANGE)
    def_die("immediate out of range");
  return neg ? -v : v;
}

static void def_parse_op(const char *s, OPERAND *k, uint64_t *imm, rel *r,
                         char *name) {
  *k = OPERAND_NULL;
  *imm = 0;
  memset(r, 0, sizeof *r);
  name[0] = 0;
  uint64_t idx;
  if (def_is_reg(s, &idx)) {
    *k = OPERAND_REG;
    *imm = idx;
    return;
  }
  if (def_is_imm(s)) {
    *k = OPERAND_IMM;
    *imm = (uint64_t)def_imm(s);
    return;
  }
  *k = OPERAND_REL;
  if (!def_valid_lab(s)) {
    def_die2("invalid operand", s);
  }
  snprintf(name, DEF_MAX_NAME, "%s", s);
  r->name = name;
}

static void def_write(const void *p, size_t n) {
  if (def_sec == SECTION_BSS) {
    *def_ctx->bss_size += n;
    return;
  }
  if (def_sec == SECTION_TEXT)
    def_die("data in .text not supported — use .rodata / .data / .bss");
  FILE *f = def_secfile();
  if (!f || fwrite(p, 1, n, f) != n)
    def_die("data stream write failed");
}

static int def_dirname(const char *s, const char *want) {
  if (s[0] == '.')
    s++;
  return strcmp(s, want) == 0;
}

static int def_is_dir(const char *s, const char *want) {
  if (s[0] == '.')
    s++;
  size_t n = strlen(want);
  if (strncmp(s, want, n) != 0)
    return 0;
  return s[n] == 0 || s[n] == ' ' || s[n] == '\t';
}

static void def_directive(char *line) {
  char *sp = line;
  while (*sp && *sp != ' ' && *sp != '\t')
    sp++;
  char saved = *sp;
  *sp = 0;
  char *rest = saved ? def_trim(sp + 1) : (char *)"";
  char args[8][DEF_MAX_LINE];
  int na = rest[0] ? def_split_ops(rest, args, 8) : 0;
  if (def_dirname(line, "string") || def_dirname(line, "constr")) {
    if (na < 1)
      def_die("string/constr requires a quoted literal");
    if (def_sec == SECTION_BSS)
      def_die("string has no data in .bss, use resb");
    char buf[DEF_MAX_LINE];
    int n = def_parse_str(args[0], buf, sizeof buf);
    buf[n] = 0;
    def_write(buf, (size_t)n + 1);
    return;
  }
  if (def_dirname(line, "resb")) {
    if (na < 1)
      def_die("resb requires a count");
    int64_t n = def_imm(args[0]);
    if (n < 0)
      def_die("negative resb");
    if (def_sec == SECTION_BSS) {
      *def_ctx->bss_size += (uint64_t)n;
      return;
    }
    uint8_t z = 0;
    for (int64_t i = 0; i < n; i++)
      def_write(&z, 1);
    return;
  }
  if (def_dirname(line, "ascii") || def_dirname(line, "asciz") ||
      def_dirname(line, "byte") || def_dirname(line, "zero") ||
      def_dirname(line, "skip"))
    def_die("removed: use  string / constr / resb");
  def_die("unknown directive");
}

static void def_instr_line(char *line, encode enc) {
  char *sp = line;
  while (*sp && *sp != ' ' && *sp != '\t')
    sp++;
  char saved = *sp;
  *sp = 0;
  char *rest = saved ? def_trim(sp + 1) : (char *)"";
  WIDTH width = WIDTH_NULL;
  if (rest[0]) {
    char *wsp = rest;
    while (*wsp && *wsp != ' ' && *wsp != '\t')
      wsp++;
    char wch = *wsp;
    *wsp = 0;
    if (strcmp(rest, "byte") == 0)
      width = WIDTH_BYTE;
    else if (strcmp(rest, "word") == 0)
      width = WIDTH_WORD;
    else if (strcmp(rest, "dword") == 0)
      width = WIDTH_DWORD;
    else if (strcmp(rest, "qword") == 0)
      width = WIDTH_QWORD;
    if (width != WIDTH_NULL)
      rest = wch ? def_trim(wsp + 1) : (char *)"";
    else
      *wsp = wch;
  }
  def_need_ins(def_nins + 1);
  def_instr *in = &def_ins[def_nins];
  memset(in, 0, sizeof *in);
  snprintf(in->mn, sizeof in->mn, "%s", line);
  in->width = width;
  in->line = def_lineno;
  char ops[4][DEF_MAX_LINE];
  int nops = rest[0] ? def_split_ops(rest, ops, 4) : 0;
  if (nops > 2)
    def_die("too many operands");
  if (nops >= 1)
    def_parse_op(ops[0], &in->oa, &in->ia, &in->ra, in->name_a);
  if (nops >= 2)
    def_parse_op(ops[1], &in->ob, &in->ib, &in->rb, in->name_b);
  in->off = *def_ctx->current_text_adrs;
  masm_line = in->line;
  FILE *save = def_ctx->text_stream;
  def_ctx->text_stream = NULL;
  enc(def_ctx, in->mn, in->width, in->oa, in->ia, in->ra, in->ob, in->ib,
      in->rb);
  def_ctx->text_stream = save;
  def_nins++;
}

static int def_set_section(const char *p) {
  if (strcmp(p, ".text") == 0 || strcmp(p, "text") == 0)
    def_sec = SECTION_TEXT;
  else if (strcmp(p, ".rodata") == 0 || strcmp(p, "rodata") == 0)
    def_sec = SECTION_RODATA;
  else if (strcmp(p, ".data") == 0 || strcmp(p, "data") == 0)
    def_sec = SECTION_DATA;
  else if (strcmp(p, ".bss") == 0 || strcmp(p, "bss") == 0)
    def_sec = SECTION_BSS;
  else
    return 0;
  return 1;
}

static void def_one_line(encode enc) {
  def_strip_comment(def_line);
  char *s = def_trim(def_line);
  if (!*s)
    return;
  char lab[DEF_MAX_NAME];
  lab[0] = 0;
  s = def_split_lab(s, lab);
  if (lab[0]) {
    def_def_lab(lab);
    def_need_ins(def_nins + 1);
    def_instr *pin = &def_ins[def_nins++];
    memset(pin, 0, sizeof *pin);
    snprintf(pin->mn, sizeof pin->mn, ".lbl %s", lab);
    pin->line = def_lineno;
    rel z;
    memset(&z, 0, sizeof z);
    FILE *save = def_ctx->text_stream;
    def_ctx->text_stream = NULL;
    enc(def_ctx, pin->mn, WIDTH_NULL, OPERAND_NULL, 0, z, OPERAND_NULL, 0, z);
    def_ctx->text_stream = save;
  }
  if (!*s)
    return;
  if (strncmp(s, ".section", 8) == 0 || strncmp(s, "section", 7) == 0) {
    char *p = *s == '.' ? def_trim(s + 8) : def_trim(s + 7);
    if (!def_set_section(p))
      def_die("unknown section");
    return;
  }
  if (def_set_section(s) || (*s == '.' && def_set_section(s + 1)))
    return;
  if (strncmp(s, ".globl", 6) == 0 || strncmp(s, ".global", 7) == 0 ||
      strncmp(s, "globl", 5) == 0 || strncmp(s, "global", 6) == 0)
    return;
  if (s[0] == '.' || def_is_dir(s, "string") || def_is_dir(s, "constr") ||
      def_is_dir(s, "resb") || def_is_dir(s, "ascii")) {
    char tmp[DEF_MAX_LINE];
    snprintf(tmp, sizeof tmp, "%s", s);
    def_directive(tmp);
    return;
  }
  if (def_sec != SECTION_TEXT)
    def_die("instruction outside .text");
  def_instr_line(s, enc);
}

static void def_resolve(def_instr *in, int which) {
  OPERAND *k = which ? &in->ob : &in->oa;
  rel *r = which ? &in->rb : &in->ra;
  char *name = which ? in->name_b : in->name_a;
  if (*k != OPERAND_REL)
    return;
  int i = def_find_lab(name);
  if (i < 0 || !def_labs[i].defined) {
    r->section = SECTION_NULL;
    r->adrs = 0;
    r->addend = 0;
    return;
  }
  r->section = def_labs[i].section;
  r->adrs = def_labs[i].off;
  r->addend = (int64_t)def_labs[i].off;
}

static void default_parse(context *ctx, char *input_file) {
  registry_object *encobj = find_registry((char *)(ctx->arch ? ctx->arch : "x86_64"));
  if (!encobj || encobj->kind != REGISTRY_ENCODER || !encobj->encoder) {
    fprintf(stderr, "masm: no encoder registered for arch '%s'\n",
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
  def_cur[0] = 0;
  def_slurp(input_file);
  def_src[def_src_len] = 0;
  const char *p = def_src;
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
  *ctx->current_text_adrs = 0;
  fseek(ctx->text_stream, 0, SEEK_SET);
  for (int i = 0; i < def_nins; i++) {
    def_instr *in = &def_ins[i];
    if (in->mn[0] == '.')
      continue;
    if (in->oa == OPERAND_REL && in->name_a[0]) {
      def_lineno = in->line;
      if (!def_valid_lab(in->name_a)) {
        fprintf(stderr, "masm: line %u: invalid operand '%s'\n", def_lineno,
                in->name_a);
        exit(1);
      }
      if (def_find_lab(in->name_a) < 0) {
        fprintf(stderr, "masm: line %u: undefined label '%s'\n", def_lineno,
                in->name_a);
        exit(1);
      }
    }
    if (in->ob == OPERAND_REL && in->name_b[0]) {
      def_lineno = in->line;
      if (!def_valid_lab(in->name_b)) {
        fprintf(stderr, "masm: line %u: invalid operand '%s'\n", def_lineno,
                in->name_b);
        exit(1);
      }
      if (def_find_lab(in->name_b) < 0) {
        fprintf(stderr, "masm: line %u: undefined label '%s'\n", def_lineno,
                in->name_b);
        exit(1);
      }
    }
  }
  for (int i = 0; i < def_nins; i++) {
    def_lineno = def_ins[i].line;
    masm_line = def_ins[i].line;
    def_resolve(&def_ins[i], 0);
    def_resolve(&def_ins[i], 1);
    enc(ctx, def_ins[i].mn, def_ins[i].width, def_ins[i].oa, def_ins[i].ia,
        def_ins[i].ra, def_ins[i].ob, def_ins[i].ib, def_ins[i].rb);
  }
}

static registry_object default_object = {
    REGISTRY_PARSER, "X26", default_parse, NULL, NULL,
};

static void default_register(void) __attribute__((constructor));
static void default_register(void) { register_object(&default_object); }

#endif
