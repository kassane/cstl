/* ************************************************************************* */
#if !defined(FIO_INCLUDE_FILE) /* Dev test - ignore line */
#define FIO___DEV___           /* Development inclusion - ignore line */
#define FIO_ATOL               /* Development inclusion - ignore line */
#include "./include.h"         /* Development inclusion - ignore line */
#endif                         /* Development inclusion - ignore line */
/* *****************************************************************************




                            String <=> Number helpers



Copyright and License: see header file (000 copyright.h) or top of file
***************************************************************************** */
#if defined(FIO_ATOL) && !defined(H___FIO_ATOL___H)
#define H___FIO_ATOL___H
#include <inttypes.h>
#include <math.h>

/* *****************************************************************************
Strings to Signed Numbers - API
***************************************************************************** */
/**
 * A helper function that converts between String data to a signed int64_t.
 *
 * Numbers are assumed to be in base 10. Octal (`0###`), Hex (`0x##`/`x##`) and
 * binary (`0b##`/ `b##`) are recognized as well. For binary Most Significant
 * Bit must come first.
 *
 * The most significant difference between this function and `strtol` (aside of
 * API design), is the added support for binary representations.
 */
SFUNC int64_t fio_atol(char **pstr);

/** A helper function that converts between String data to a signed double. */
SFUNC double fio_atof(char **pstr);

/* *****************************************************************************
Signed Numbers to Strings - API
***************************************************************************** */

/**
 * A helper function that writes a signed int64_t to a string.
 *
 * No overflow guard is provided, make sure there's at least 68 bytes available
 * (for base 2).
 *
 * Offers special support for base 2 (binary), base 8 (octal), base 10 and base
 * 16 (hex) where prefixes are automatically added if required (i.e.,`"0x"` for
 * hex and `"0b"` for base 2, and `"0"` for octal).
 *
 * Supports any base up to base 36 (using 0-9,A-Z).
 *
 * An unsupported base will log an error and print zero.
 *
 * Returns the number of bytes actually written (excluding the NUL terminator).
 */
SFUNC size_t fio_ltoa(char *dest, int64_t num, uint8_t base);

/**
 * A helper function that converts between a double to a string.
 *
 * No overflow guard is provided, make sure there's at least 130 bytes
 * available (for base 2).
 *
 * Supports base 2, base 10 and base 16. An unsupported base will silently
 * default to base 10. Prefixes aren't added (i.e., no "0x" or "0b" at the
 * beginning of the string).
 *
 * Returns the number of bytes actually written (excluding the NUL
 * terminator).
 */
SFUNC size_t fio_ftoa(char *dest, double num, uint8_t base);

/* *****************************************************************************
Unsigned Numbers, Building Blocks and Helpers
***************************************************************************** */
/**
 * Maps characters to alphanumerical value, where numbers have their natural
 * values (0-9) and `A-Z` (or `a-z`) are the values 10-35.
 *
 * Out of bound values return 255.
 *
 * This allows parsing of numeral strings for up to base 36.
 */
IFUNC uint8_t fio_c2i(unsigned char c);

/**
 * Maps numeral values to alphanumerical characters, where numbers have their
 * natural values (0-9) and `A-Z` are the values 10-35.
 *
 * Accepts values up to 63. Returns zero for values over 35. Out of bound values
 * produce undefined behavior.
 *
 * This allows printing of numerals for up to base 36.
 */
IFUNC uint8_t fio_i2c(unsigned char i);

/** Returns the number of digits in base 10. */
FIO_IFUNC size_t fio_digits10(int64_t i);
/** Returns the number of digits in base 10 for an unsigned number. */
FIO_SFUNC size_t fio_digits10u(uint64_t i);

/** Returns the number of digits in base 8 for an unsigned number. */
FIO_SFUNC size_t fio_digits8u(uint64_t i);
/** Returns the number of digits in base 16 for an unsigned number. */
FIO_SFUNC size_t fio_digits16u(uint64_t i);
/** Returns the number of digits in base 2 for an unsigned number. */
FIO_SFUNC size_t fio_digits_bin(uint64_t i);
/** Returns the number of digits in any base X<65 for an unsigned number. */
FIO_SFUNC size_t fio_digits_xbase(uint64_t i, size_t base);

/** Writes a signed number to `dest` using `digits` bytes (+ `NUL`) */
FIO_IFUNC void fio_ltoa10(char *dest, int64_t i, size_t digits);
/** Reads a signed base 10 formatted number. */
SFUNC int64_t fio_atol10(char **pstr);

/** Writes unsigned number to `dest` using `digits` bytes (+ `NUL`) */
FIO_IFUNC void fio_ltoa10u(char *dest, uint64_t i, size_t digits);
/** Writes unsigned number to `dest` using `digits` bytes (+ `NUL`) */
FIO_IFUNC void fio_ltoa16u(char *dest, uint64_t i, size_t digits);
/** Writes unsigned number to `dest` using `digits` bytes (+ `NUL`) */
FIO_IFUNC void fio_ltoa_bin(char *dest, uint64_t i, size_t digits);
/** Writes unsigned number to `dest` using `digits` bytes (+ `NUL`) */
FIO_IFUNC void fio_ltoa_xbase(char *dest,
                              uint64_t i,
                              size_t digits,
                              size_t base);

/** Reads a signed base 8 formatted number. */
SFUNC uint64_t fio_atol8u(char **pstr);
/** Reads a signed base 10 formatted number. */
SFUNC uint64_t fio_atol10u(char **pstr);
/** Reads an unsigned hex formatted number (possibly prefixed with "0x"). */
SFUNC uint64_t fio_atol16u(char **pstr);
/** Reads an unsigned binary formatted number (possibly prefixed with "0b"). */
SFUNC uint64_t fio_atol_bin(char **pstr);
/** Read an unsigned number in any base up to base 36. */
SFUNC uint64_t fio_atol_xbase(char **pstr, size_t base);

/** Converts an unsigned `val` to a signed `val`, with overflow protection. */
FIO_IFUNC int64_t fio_u2i_limit(uint64_t val, size_t invert);

/* *****************************************************************************
IEEE 754 Floating Points, Building Blocks and Helpers
***************************************************************************** */

/** Converts a 64 bit integer to an IEEE 754 formatted double. */
FIO_IFUNC double fio_i2d(int64_t mant, int64_t exponent);

/* *****************************************************************************


Implementation - inlined


***************************************************************************** */

/** Returns the number of digits in base 10. */
FIO_IFUNC size_t fio_digits10(int64_t i) {
  if (i >= 0)
    return fio_digits10u(i);
  return fio_digits10u((0ULL - (uint64_t)i)) + 1;
}

/** Returns the number of digits in base 2 for an unsigned number. */
FIO_SFUNC size_t fio_digits_bin(uint64_t i) {
  size_t r = 1;
  if (!i)
    return r;
  r = fio_msb_index_unsafe(i) + 1;
  r += (r & 1); /* binary is written 2 zeros at a time */
  return r;
}

/** Returns the number of digits in base 8 for an unsigned number. */
FIO_SFUNC size_t fio_digits8u(uint64_t i) {
  size_t r = 1;
  for (;;) {
    if (i < 8)
      return r;
    if (i < 64)
      return r + 1;
    if (i < 512)
      return r + 2;
    if (i < 4096)
      return r + 3;
    if (i < 32768)
      return r + 4;
    if (i < 262144)
      return r + 5;
    if (i < 2097152)
      return r + 6;
    if (i < 16777216)
      return r + 7;
    r += 8;
    i >>= 24;
  }
}

/** Returns the number of digits in base 10 for an unsigned number. */
FIO_SFUNC size_t fio_digits10u(uint64_t i) {
  size_t r = 1;
  for (;;) {
    if (i < 10ULL)
      return r;
    if (i < 100ULL)
      return r + 1;
    if (i < 1000ULL)
      return r + 2;
    if (i < 10000ULL)
      return r + 3;
    r += 4;
    i /= 10000ULL;
  }
}

/** Returns the number of digits in base 16 for an unsigned number. */
FIO_SFUNC size_t fio_digits16u(uint64_t i) {
  if (i < 0x100ULL)
    return 2;
  if (i < 0x10000ULL)
    return 4;
  if (i < 0x1000000ULL)
    return 6;
  if (i < 0x100000000ULL)
    return 8;
  if (i < 0x10000000000ULL)
    return 10;
  if (i < 0x1000000000000ULL)
    return 12;
  if (i < 0x100000000000000ULL)
    return 14;
  return 16;
}

/** Returns the number of digits in base X<65 for an unsigned number. */
FIO_SFUNC size_t fio_digits_xbase(uint64_t i, size_t base) {
  size_t base2 = base * base;
  size_t base3 = base2 * base;
  size_t base4 = base3 * base;
  size_t r = 1;
  for (;;) {
    if (i < base)
      return r;
    if (i < base2)
      return r + 1;
    if (i < base3)
      return r + 2;
    if (i < base4)
      return r + 3;
    r += 4;
    i /= base4;
  }
}

FIO_IFUNC void fio_ltoa10(char *dest, int64_t i, size_t digits) {
  size_t inv = i < 0;
  dest[0] = '-';
  dest += inv;
  if (inv)
    i = (int64_t)((uint64_t)0 - (uint64_t)i);
  fio_ltoa10u(dest, (uint64_t)i, digits - inv);
}

FIO_IFUNC void fio_ltoa8u(char *dest, uint64_t i, size_t digits) {
  dest += digits;
  *dest-- = 0;
  while (i > 7) {
    *dest-- = '0' + (i & 7);
    i >>= 3;
  }
  *dest = '0' + i;
}

FIO_IFUNC void fio_ltoa10u(char *dest, uint64_t i, size_t digits) {
  dest += digits;
  *dest-- = 0;
  while (i > 9) {
    uint64_t nxt = i / 10;
    *dest-- = '0' + (i - (nxt * 10ULL));
    i = nxt;
  }
  *dest = '0' + (unsigned char)i;
}

FIO_IFUNC void fio_ltoa16u(char *dest, uint64_t i, size_t digits) {
  dest += digits;
  *dest-- = 0;
  while (i > 255) {
    *dest-- = fio_i2c(i & 15);
    i >>= 4;
    *dest-- = fio_i2c(i & 15);
    i >>= 4;
  }
  *dest-- = fio_i2c(i & 15);
  i >>= 4;
  *dest = fio_i2c(i);
}

FIO_IFUNC void fio_ltoa_bin(char *dest, uint64_t i, size_t digits) {
  dest += digits;
  *dest-- = 0;
  switch (digits & 7) { /* last use of `digits` */
    while (i) {
      *dest-- = '0' + (i & 1);
      i >>= 1;                                /* fall through */
    case 7: *dest-- = '0' + (i & 1); i >>= 1; /* fall through */
    case 6: *dest-- = '0' + (i & 1); i >>= 1; /* fall through */
    case 5: *dest-- = '0' + (i & 1); i >>= 1; /* fall through */
    case 4: *dest-- = '0' + (i & 1); i >>= 1; /* fall through */
    case 3: *dest-- = '0' + (i & 1); i >>= 1; /* fall through */
    case 2: *dest-- = '0' + (i & 1); i >>= 1; /* fall through */
    case 1: *dest-- = '0' + (i & 1); i >>= 1; /* fall through */
    case 0:;
    }
  }
}

FIO_IFUNC void fio_ltoa_xbase(char *dest,
                              uint64_t i,
                              size_t digits,
                              size_t base) {
  dest += digits;
  *dest-- = 0;
  while (i >= base) {
    uint64_t nxt = i / base;
    *dest-- = fio_i2c(i - (nxt * base));
    i = nxt;
  }
  *dest = fio_i2c(i);
}

/** Converts an unsigned `val` to a signed `val`, with overflow protection. */
FIO_IFUNC int64_t fio_u2i_limit(uint64_t val, size_t to_negative) {
  if (!to_negative) {
    /* overflow? */
    if (!(val & 0x8000000000000000ULL))
      return val;
    errno = E2BIG;
    val = 0x7FFFFFFFFFFFFFFFULL;
    return val;
  }
  if (!(val & 0x8000000000000000ULL)) {
    val = (int64_t)0LL - (int64_t)val;
    return val;
  }
  /* read overflow */
  errno = E2BIG;
  return (int64_t)(val = 0x8000000000000000ULL);
}

/* *****************************************************************************
IEEE 754 Floating Points, Building Blocks and Helpers
***************************************************************************** */

#ifndef FIO_MATH_DBL_MANT_MASK
#define FIO_MATH_DBL_MANT_MASK (((uint64_t)1ULL << 52) - 1)
#define FIO_MATH_DBL_EXPO_MASK ((uint64_t)2047ULL << 52)
#define FIO_MATH_DBL_SIGN_MASK ((uint64_t)1ULL << 63)
#endif

/** Converts a 64 bit integer to an IEEE 754 formatted double. */
FIO_IFUNC double fio_i2d(int64_t mant, int64_t exponent) {
  union {
    uint64_t u64;
    double d;
  } u = {.u64 = ((uint64_t)(mant)&FIO_MATH_DBL_SIGN_MASK)};
  size_t tmp;
  if (!mant)
    goto is_zero;
  /* convert `mant` to absolute value - constant time */
  tmp = u.u64 >> 63;
  mant =
      (int64_t)(uint64_t)mant ^
      (((uint64_t)0 - tmp) & ((uint64_t)mant ^ (uint64_t)((int64_t)0 - mant)));
  // mant = (int64_t)(((uint64_t)mant ^ ((uint64_t)0 - tmp)) + tmp); // slower
  /* normalize exponent */
  tmp = fio_msb_index_unsafe(mant);
  exponent += tmp + 1023;
  if (FIO_UNLIKELY(exponent < 0))
    goto is_zero;
  if (FIO_UNLIKELY(exponent > 2047))
    goto is_inifinity_or_nan;
  exponent = (uint64_t)exponent << 52;
  u.u64 |= exponent;
  /* reposition mant bits so we "hide" the fist set bit in bit[52] */
  if (tmp < 52)
    mant = mant << (52 - tmp);
  else if (FIO_UNLIKELY(tmp > 52)) /* losing precision */
    mant = mant >> (tmp - 52);
  u.u64 |= mant & FIO_MATH_DBL_MANT_MASK; /* remove the 1 set bit */
is_zero:
  return u.d;
is_inifinity_or_nan:
  u.u64 |= FIO_MATH_DBL_EXPO_MASK | (mant & FIO_MATH_DBL_MANT_MASK);
  return u.d;
}

/* *****************************************************************************
Implementation - possibly externed
***************************************************************************** */
#if defined(FIO_EXTERN_COMPLETE) || !defined(FIO_EXTERN)

typedef struct {
  uint64_t val;
  int64_t expo;
  uint8_t sign;
} fio___number_s;

/* *****************************************************************************
Unsigned core and helpers
***************************************************************************** */

/**
 * Maps characters to alphanumerical value, where numbers have their natural
 * values (0-9) and `A-Z` (or `a-z`) are the values 10-35.
 *
 * Out of bound values return 255.
 *
 * This allows parsing of numeral strings for up to base 36.
 */
IFUNC uint8_t fio_c2i(unsigned char c) {
  static const uint8_t fio___alphanumeric_map[256] = {
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   255, 255,
      255, 255, 255, 255, 255, 10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
      20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,
      35,  255, 255, 255, 255, 255, 255, 10,  11,  12,  13,  14,  15,  16,  17,
      18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
      33,  34,  35,  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255};
  return fio___alphanumeric_map[c];
}

/**
 * Maps numeral values to alphanumerical characters, where numbers have their
 * natural values (0-9) and `A-Z` are the values 10-35.
 *
 * Accepts values up to 63. Returns zero for values over 35. Out of bound values
 * produce undefined behavior.
 *
 * This allows printing of numerals for up to base 36.
 */
IFUNC uint8_t fio_i2c(unsigned char i) {
  static const uint8_t fio___alphanumeric_map[64] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
      'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
      'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
  return fio___alphanumeric_map[i & 63];
}

/** Reads a signed base 10 formatted number. */
SFUNC int64_t fio_atol10(char **pstr) {
  const uint64_t add_limit = (~(uint64_t)0ULL) - 9;
  char *pos = *pstr;
  const size_t inv = (pos[0] == '-');
  pos += inv;
  uint64_t val = 0;
  uint64_t r0;
  while (((r0 = pos[0] - '0') < 10ULL) & (val < add_limit)) {
    val *= 10;
    val += r0;
    ++pos;
  }
  if (((size_t)(pos[0] - '0') < 10ULL)) {
    errno = E2BIG;
  }
  *pstr = pos;
  return fio_u2i_limit(val, inv);
}

/** Reads a signed base 8 formatted number. */
SFUNC uint64_t fio_atol8u(char **pstr) {
  uint64_t r = 0;
  size_t d;
  while ((d = (size_t)fio_c2i((unsigned char)(**pstr))) < 8) {
    r <<= 3;
    r |= d;
    ++*pstr;
    if ((r & UINT64_C(0xE000000000000000)))
      break;
  }
  if ((fio_c2i(**pstr)) < 8)
    errno = E2BIG;
  return r;
}

/** Reads an unsigned base 10 formatted number. */
SFUNC uint64_t fio_atol10u(char **pstr) {
  uint64_t r = 0;
  const uint64_t add_limit = (~(uint64_t)0ULL) - 9;
  char *pos = *pstr;
  uint64_t r0;
  while (((r0 = (uint64_t)(pos[0] - '0')) < 10ULL) & (r < add_limit)) {
    r *= 10;
    r += r0;
    ++pos;
  }
  while (((size_t)(pos[0] - '0') < 10ULL)) {
    errno = E2BIG;
    ++pos;
  }
  *pstr = pos;
  return r;
}

/** Reads an unsigned hex formatted number (possibly prefixed with "0x"). */
SFUNC uint64_t fio_atol16u(char **pstr) {
  uint64_t r = 0;
  size_t d;
  unsigned char *p = (unsigned char *)*pstr;
  p += ((p[0] == '0') & ((p[1] | 32) == 'x')) << 1;
  if ((d = fio_c2i(*p)) > 15)
    goto possible_misread;
  for (;;) {
    r |= d;
    ++p;
    d = (size_t)fio_c2i(*p);
    if (d > 15)
      break;
    if ((r & UINT64_C(0xF000000000000000))) {
      errno = E2BIG;
      break;
    }
    r <<= 4;
  }
  *pstr = (char *)p;
  return r;
possible_misread:
  /* if 0x was read, move to X. */
  *pstr += ((pstr[0][0] == '0') & ((pstr[0][1] | 32) == 'x'));
  return r;
}

/** Reads an unsigned binary formatted number (possibly prefixed with "0b"). */
SFUNC uint64_t fio_atol_bin(char **pstr) {
  uint64_t r = 0;
  size_t d;
  *pstr += (**pstr == '0');
  *pstr += (**pstr | 32) == 'b' && ((size_t)(pstr[0][1]) - (size_t)'0') < 2;
  while ((d = (size_t)((unsigned char)(**pstr)) - (size_t)'0') < 2) {
    r <<= 1;
    r |= d;
    ++*pstr;
    if ((r & UINT64_C(0x8000000000000000)))
      break;
  }
  if ((d = (size_t)(**pstr) - (size_t)'0') < 2)
    errno = E2BIG;
  return r;
}

/** Attempts to read an unsigned number in any base up to base 36. */
SFUNC uint64_t fio_atol_xbase(char **pstr, size_t base) {
  uint64_t r = 0;
  if (base > 36)
    return r;
  if (base == 10)
    return (r = fio_atol10u(pstr));
  if (base == 16)
    return (r = fio_atol16u(pstr));
  if (base == 2)
    return (r = fio_atol_bin(pstr));
  const uint64_t limit = (~UINT64_C(0)) / base;
  size_t d;
  while ((d = (size_t)fio_c2i((unsigned char)(**pstr))) < base) {
    r *= base;
    r += d;
    ++*pstr;
    if (r > limit)
      break;
  }
  if ((fio_c2i(**pstr)) < base)
    errno = E2BIG;
  return r;
}

/* *****************************************************************************
fio_atol
***************************************************************************** */

SFUNC int64_t fio_atol(char **pstr) {
  if (!pstr || !(*pstr))
    return 0;
  uint64_t v = 0;
  uint64_t (*fn)(char **) = fio_atol10u;
  char *p = *pstr;
  unsigned inv = (p[0] == '-');
  p += inv;
  char *const s = p;
  switch (*p) {
  case 'x': /* fall through */
  case 'X': fn = fio_atol16u; goto compute;
  case 'b': /* fall through */
  case 'B': fn = fio_atol_bin; goto compute;
  case '0':
    switch (p[1]) {
    case 'x': /* fall through */
    case 'X': fn = fio_atol16u; goto compute;
    case 'b': /* fall through */
    case 'B': fn = fio_atol_bin; goto compute;
    case '0': /* fall through */
    case '1': /* fall through */
    case '2': /* fall through */
    case '3': /* fall through */
    case '4': /* fall through */
    case '5': /* fall through */
    case '6': /* fall through */
    case '7': fn = fio_atol8u;
    }
  }
compute:
  v = fn(&p);
  if (p != s)
    *pstr = p;
  if (fn == fio_atol10u)
    return fio_u2i_limit(v, inv);
  if (!inv) /* sign embedded in the representation */
    return (int64_t)v;
  return fio_u2i_limit(v, inv);
}

/* *****************************************************************************
fio_ltoa
***************************************************************************** */

SFUNC size_t fio_ltoa(char *dest, int64_t num, uint8_t base) {
  size_t len = 0;
  uint64_t n = (uint64_t)num;
  size_t digits;
  char dump[96];
  if (!dest)
    dest = dump;

  switch (base) {
  case 1: /* fall through */
  case 2: /* Base 2 */
    len += (digits = fio_digits_bin(n));
    fio_ltoa_bin(dest, n, digits); /* embedded sign bit */
    return len;
  case 8: /* Base 8 */
    if (num < 0) {
      *(dest++) = '-';
      n = 0 - n;
      ++len;
    }
    len += (digits = fio_digits8u(n));
    fio_ltoa8u(dest, n, digits);
    return len;
  case 16: /* Base 16 */
    len += (digits = fio_digits16u(n));
    fio_ltoa16u(dest, n, digits); /* embedded sign bit */
    return len;
  case 0:  /* fall through */
  case 10: /* Base 10 */
    if (num < 0) {
      *(dest++) = '-';
      n = 0 - n;
      ++len;
    }
    len += (digits = fio_digits10u(n));
    fio_ltoa10u(dest, n, digits);
    return len;
  default: /* any base up to base 36 */
    if (base > 36)
      goto base_error;
    if (num < 0) {
      *(dest++) = '-';
      n = 0 - n;
      ++len;
    }
    len += (digits = fio_digits_xbase(n, base));
    fio_ltoa_xbase(dest, n, digits, base);
    return len;
  }

base_error:
  FIO_LOG_ERROR("fio_ltoa base out of range");
  return len;
}

/* *****************************************************************************
fio_atof
***************************************************************************** */

SFUNC double fio_atof(char **pstr) {
  if (!pstr || !(*pstr))
    return 0;
  if ((*pstr)[1] == 'b' || ((*pstr)[1] == '0' && (*pstr)[1] == 'b'))
    goto binary_raw;
  return strtod(*pstr, pstr);
binary_raw:
  /* binary representation is assumed to spell an exact double */
  (void)0;
  union {
    uint64_t i;
    double d;
  } punned = {.i = (uint64_t)fio_atol(pstr)};
  return punned.d;
}

/* *****************************************************************************
fio_ftoa
***************************************************************************** */

SFUNC size_t fio_ftoa(char *dest, double num, uint8_t base) {
  if (base == 2 || base == 16) {
    /* handle binary / Hex representation the same as an int64_t */
    /* FIXME: Hex representation should use floating-point hex instead */
    union {
      int64_t i;
      double d;
    } p;
    p.d = num;
    return fio_ltoa(dest, p.i, base);
  }
  size_t written = 0;

  if (isinf(num))
    goto is_inifinity;
  if (isnan(num))
    goto is_nan;

  written = snprintf(dest, 30, "%g", num);
  /* test if we need to add a ".0" to the end of the string */
  for (char *start = dest;;) {
    switch (*start) {
    case ',':
      *start = '.'; // locale issues?
    /* fall through */
    case 'e': /* fall through */
    case '.': /* fall through */ goto finish;
    case 0: goto add_dot_zero;
    }
    ++start;
  }
add_dot_zero:
  dest[written++] = '.';
  dest[written++] = '0';

finish:
  dest[written] = 0;
  return written;

is_inifinity:
  if (num < 0)
    dest[written++] = '-';
  fio_memcpy8(dest + written, "Infinity");
  written += 8;
  dest[written] = 0;
  return written;
is_nan:
  fio_memcpy4(dest, "NaN");
  return 3;
}

/* *****************************************************************************
Numbers <=> Strings - Testing
***************************************************************************** */

#ifdef FIO_TEST_ALL

#endif /* FIO_TEST_ALL */

/* *****************************************************************************
Numbers <=> Strings - Cleanup
***************************************************************************** */
#endif /* FIO_EXTERN_COMPLETE */
#endif /* FIO_ATOL */
#undef FIO_ATOL
