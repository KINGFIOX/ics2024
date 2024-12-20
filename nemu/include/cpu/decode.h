/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include <isa.h>

typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc;  // static next pc
  vaddr_t dnpc;  // dynamic next pc
  ISADecodeInfo isa;
  IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode;

#define macro2(i) \
  macro(i);       \
  macro((i) + 1)
#define macro4(i) \
  macro2(i);      \
  macro2((i) + 2)
#define macro8(i) \
  macro4(i);      \
  macro4((i) + 4)
#define macro16(i) \
  macro8(i);       \
  macro8((i) + 8)
#define macro32(i) \
  macro16(i);      \
  macro16((i) + 16)
#define macro64(i) \
  macro32(i);      \
  macro32((i) + 32)

// --- pattern matching mechanism ---

// example:
// uint64_t key, mask, shift;
//
// pattern_decode("1011 0???", STRLEN("1011 0???"), &key, &mask, &shift);
// STRLEN("1011 0???") = 9
// pat   = 0b1011_0???
//
// key   = 0b1011_1
// mask  = 0b1111_1
// shift = 3

/**
 * @brief decode the pattern string to key, mask and shift
 *
 * @param str pattern string
 * @param len length of the pattern string
 * @param key (returned)
 * @param mask (returned)
 * @param shift (returned) decoded shift (count of the '?')
 */
__attribute__((always_inline)) static inline void pattern_decode(const char *str, int len, uint64_t *key, uint64_t *mask, uint64_t *shift) {
  /* ---------- ---------- declaration begin ---------- ---------- */
#define macro(i)                                                                               \
  if ((i) >= len) {                                                                            \
    *key = __key >> __shift;                                                                   \
    *mask = __mask >> __shift;                                                                 \
    *shift = __shift;                                                                          \
    return;                                                                                    \
  } else {                                                                                     \
    char c = str[i];                                                                           \
    if (c != ' ') {                                                                            \
      Assert(c == '0' || c == '1' || c == '?', "invalid character '%c' in pattern string", c); \
      __key = (__key << 1) | (c == '1' ? 1 : 0);                                               \
      __mask = (__mask << 1) | (c == '?' ? 0 : 1);                                             \
      __shift = (c == '?' ? __shift + 1 : 0);                                                  \
    }                                                                                          \
  }
  /* ---------- ---------- declaration end ---------- ---------- */

  // actually running the macro. above are just declarations.
  uint64_t __key = 0, __mask = 0, __shift = 0;
  macro64(0);
#undef macro

  panic("pattern too long");
}

__attribute__((always_inline)) static inline void pattern_decode_hex(const char *str, int len, uint64_t *key, uint64_t *mask, uint64_t *shift) {
  /* ---------- ---------- declaration begin ---------- ---------- */
#define macro(i)                                                                                                           \
  if ((i) >= len) {                                                                                                        \
    *key = __key >> __shift;                                                                                               \
    *mask = __mask >> __shift;                                                                                             \
    *shift = __shift;                                                                                                      \
    return;                                                                                                                \
  } else {                                                                                                                 \
    char c = str[i];                                                                                                       \
    if (c != ' ') {                                                                                                        \
      Assert((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || c == '?', "invalid character '%c' in pattern string", c); \
      __key = (__key << 4) | (c == '?' ? 0 : (c >= '0' && c <= '9') ? c - '0' : c - 'a' + 10);                             \
      __mask = (__mask << 4) | (c == '?' ? 0 : 0xf);                                                                       \
      __shift = (c == '?' ? __shift + 4 : 0);                                                                              \
    }                                                                                                                      \
  }
  /* ---------- ---------- declaration end ---------- ---------- */

  // actually running the macro. above are just declarations.
  uint64_t __key = 0, __mask = 0, __shift = 0;
  macro16(0);
#undef macro

  panic("pattern too long");
}

// --- pattern matching wrappers for decode ---
#define INSTPAT(pattern, ...)                                      \
  do {                                                             \
    uint64_t key, mask, shift;                                     \
    pattern_decode(pattern, STRLEN(pattern), &key, &mask, &shift); \
    if ((((uint64_t)INSTPAT_INST(s) >> shift) & mask) == key) {    \
      INSTPAT_MATCH(s, ##__VA_ARGS__);                             \
      goto *(__instpat_end);                                       \
    }                                                              \
  } while (0)

#define INSTPAT_START(name) \
  {                         \
    const void *__instpat_end = &&concat(__instpat_end_, name);

#define INSTPAT_END(name)         \
  concat(__instpat_end_, name) :; \
  }

#endif
