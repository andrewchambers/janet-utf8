#include <janet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(JANET_BSD) && !defined(JANET_WINDOWS)
#include <alloca.h>
#endif
#ifdef JANET_WINDOWS
#include <malloc.h>
#define alloca _alloca
#endif

#include "build/utf8proc.c"

static Janet jutf8_valid(int argc, Janet *argv) {
  janet_fixarity(argc, 1);
  JanetByteView bv = janet_getbytes(argv, 0);
  const utf8proc_uint8_t *s = bv.bytes;
  utf8proc_ssize_t l = bv.len;
  utf8proc_int32_t r;
  while (l > 0) {
    utf8proc_ssize_t sz = utf8proc_iterate(s, l, &r);
    if (r == -1)
      return janet_wrap_boolean(0);
    l -= sz;
    s += sz;
  }
  return janet_wrap_boolean(1);
}

static inline Janet jutf8_conv_case(int argc, Janet *argv, int up) {
  janet_fixarity(argc, 1);
  JanetByteView bv = janet_getbytes(argv, 0);
  const utf8proc_uint8_t *s = bv.bytes;
  utf8proc_ssize_t l = bv.len;
  utf8proc_ssize_t bytes_converted = 0;
  utf8proc_int32_t r = 0;
  const utf8proc_ssize_t alloca_limit = 64;
  utf8proc_uint8_t *s2 = l > alloca_limit ? janet_smalloc(l) : alloca(l);
  for (utf8proc_ssize_t i = 0; i < l;) {
    utf8proc_ssize_t sz = utf8proc_iterate(s + i, l - i, &r);
    if (r == -1)
      janet_panicf("%v is not a valid utf8 string: %s", argv[0],
                   utf8proc_errmsg(sz));
    i += sz;
    utf8proc_uint8_t buf[4];
    utf8proc_ssize_t n = utf8proc_encode_char(
        up ? utf8proc_toupper(r) : utf8proc_tolower(r), buf);
    if (n > sz) {
      // Fairly sure this holds true.
      janet_panicf("janet-utf8-bug: utf8 case conversion increased string "
                   "length (file an issue)");
    }
    for (utf8proc_ssize_t j = 0; j < n; j++) {
      s2[bytes_converted++] = buf[j];
    }
  }
  Janet v = janet_stringv(s2, bytes_converted);
  if (l > alloca_limit) {
    janet_sfree(s2);
  }
  return v;
}

static Janet jutf8_to_lower(int argc, Janet *argv) {
  return jutf8_conv_case(argc, argv, 0);
}

static Janet jutf8_to_upper(int argc, Janet *argv) {
  return jutf8_conv_case(argc, argv, 1);
}

static Janet jutf8_to_codepoints(int argc, Janet *argv) {
  janet_fixarity(argc, 1);
  JanetByteView bv = janet_getbytes(argv, 0);
  const utf8proc_uint8_t *s = bv.bytes;
  utf8proc_ssize_t l = bv.len;
  utf8proc_int32_t r = 0;
  JanetArray *a = janet_array(l);
  for (utf8proc_ssize_t i = 0; i < l;) {
    utf8proc_ssize_t sz = utf8proc_iterate(s + i, l - i, &r);
    if (r == -1)
      janet_panicf("%v is not a valid utf8 string: %s", argv[0],
                   utf8proc_errmsg(sz));
    i += sz;
    janet_array_push(a, janet_wrap_number(r));
  }
  return janet_wrap_array(a);
}

static inline Janet jutf8_normalize2(int argc, Janet *argv,
                                     utf8proc_option_t options) {
  janet_fixarity(argc, 1);
  JanetByteView bv = janet_getbytes(argv, 0);
  Janet v;
  utf8proc_uint8_t *s = NULL;
  utf8proc_int32_t slen =
      utf8proc_map(bv.bytes, bv.len, &s, options);
  if (slen < 0)
    janet_panicf("utf8 normalization failure: %s", utf8proc_errmsg(slen));
  v = janet_stringv(s, slen);
  free(s);
  return v;
}

static Janet jutf8_normalize_NFC(int argc, Janet *argv) {
  return jutf8_normalize2(argc, argv, UTF8PROC_STABLE | UTF8PROC_COMPOSE);
}

static Janet jutf8_normalize_NFD(int argc, Janet *argv) {
  return jutf8_normalize2(argc, argv, UTF8PROC_STABLE | UTF8PROC_DECOMPOSE);
}

static Janet jutf8_normalize_NFKD(int argc, Janet *argv) {
  return jutf8_normalize2(
      argc, argv, UTF8PROC_STABLE | UTF8PROC_DECOMPOSE | UTF8PROC_COMPAT);
}

static Janet jutf8_normalize_NFKC(int argc, Janet *argv) {
  return jutf8_normalize2(argc, argv,
                          UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_COMPAT);
}

static Janet jutf8_normalize_NFKC_Casefold(int argc, Janet *argv) {
  return jutf8_normalize2(argc, argv,
                          UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_COMPAT |
                              UTF8PROC_CASEFOLD | UTF8PROC_IGNORE);
}

static const JanetReg cfuns[] = {
    {"valid?", jutf8_valid,
     "(utf8/valid? s)\n\nReturn true if s is a valid utf8 character sequence, "
     "otherwise false."},
    {"to-lower", jutf8_to_lower,
     "(utf8/to-lower s)\n\nReturn the lower case conversion of s or raise an "
     "error if s invalid utf8."},
    {"to-upper", jutf8_to_upper,
     "(utf8/to-upper s)\n\nReturn the upper case conversion of s or raise an "
     "error if s invalid utf8."},
    {"to-codepoints", jutf8_to_codepoints,
     "(utf8/to-codepoints s)\n\nReturn an array containing s split into "
     "codepoints as janet numbers."},
    {"normalize", jutf8_normalize_NFC,
     "(utf8/normalize s)\n\nReturn the utf8 NFC normalization of s."},
    {"normalize-NFC", jutf8_normalize_NFC,
     "(utf8/normalize-NFC s)\n\nReturn the utf8 NFC normalization of s."},
    {"normalize-NFD", jutf8_normalize_NFD,
     "(utf8/normalize-NFD s)\n\nReturn the utf8 NFD normalization of s."},
    {"normalize-NFKD", jutf8_normalize_NFKD,
     "(utf8/normalize-NFKD s)\n\nReturn the utf8 NFKD normalization of s."},
    {"normalize-NFKC", jutf8_normalize_NFKC,
     "(utf8/normalize-NFKC s)\n\nReturn the utf8 NFKC normalization of s."},
    {"normalize-NFKC-casefold", jutf8_normalize_NFKC_Casefold,
     "(utf8/normalize-NFKC-casefold s)\n\nReturn the utf8 NFKC-casefold "
     "normalization of s."},
    {NULL, NULL, NULL}};

JANET_MODULE_ENTRY(JanetTable *env) { janet_cfuns(env, "utf8", cfuns); }
