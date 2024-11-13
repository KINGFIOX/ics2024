#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) { panic("Not implemented"); }

char *strcpy(char *dst, const char *src) { panic("Not implemented"); }

char *strncpy(char *dst, const char *src, size_t n) { panic("Not implemented"); }

char *strcat(char *dst, const char *src) { panic("Not implemented"); }

int strcmp(const char *p, const char *q) {
  while (*p && *p == *q) p++, q++;
  return (uint8_t)*p - (uint8_t)*q;
}

int strncmp(const char *s1, const char *s2, size_t n) { panic("Not implemented"); }

void *memset(void *s, int c, size_t n) { panic("Not implemented"); }

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

int memcmp(const void *s1, const void *s2, size_t n) { panic("Not implemented"); }

#endif
