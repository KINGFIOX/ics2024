#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t n;

  for (n = 0; s[n]; n++);
  return n;
}

char *strcpy(char *dst, const char *src) {
  char *os;

  os = dst;
  while ((*dst++ = *src++) != 0);
  return os;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *os;

  os = dst;
  while (n-- > 0 && (*dst++ = *src++) != 0);
  while (n-- > 0) *dst++ = 0;
  return os;
}

char *strcat(char *dst, const char *src) {
  char *os;

  os = dst;
  while (*dst) dst++;
  while ((*dst++ = *src++) != 0);
  return os;
}

int strcmp(const char *p, const char *q) {
  while (*p && *p == *q) p++, q++;
  return (uint8_t)*p - (uint8_t)*q;
}

int strncmp(const char *p, const char *q, size_t n) {
  while (n > 0 && *p && *p == *q) n--, p++, q++;
  if (n == 0) return 0;
  return (uint8_t)*p - (uint8_t)*q;
}

void *memset(void *dst, int c, size_t n) {
  char *cdst = (char *)dst;
  for (size_t i = 0; i < n; i++) {
    cdst[i] = c;
  }
  return dst;
}

void *memmove(void *vdst, const void *vsrc, size_t n) {
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  if (src > dst) {
    while (n-- > 0) *dst++ = *src++;
  } else {
    dst += n;
    src += n;
    while (n-- > 0) *--dst = *--src;
  }
  return vdst;
}

void *memcpy(void *dst, const void *src, size_t n) { return memmove(dst, src, n); }

int memcmp(const void *v1, const void *v2, size_t n) {
  const uint8_t *s1, *s2;

  s1 = v1;
  s2 = v2;
  while (n-- > 0) {
    if (*s1 != *s2) return *s1 - *s2;
    s1++, s2++;
  }

  return 0;
}

#endif
