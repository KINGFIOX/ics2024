#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUF_SIZE (4096)

#define PUTCH_DEBUG 0

static inline int sputc(char *s, char c) {  // NOTE: ⭐
  *s = c;
  return 1;
}

static const char digits[] = "0123456789abcdef";

static inline int sprintint(char *s, int xx, int base, int sign) {  // NOTE: ⭐
  char buf[16];
  memset(buf, 0, 16);

  uint32_t x;
  if (sign && (sign = xx < 0)) {
    x = -xx;
  } else {
    x = xx;
  }

  int i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sign) {
    buf[i++] = '-';
  }

  size_t n = 0;
  while (--i >= 0) {
    n += sputc(s + n, buf[i]);
  }

#if PUTCH_DEBUG
  for (char *p = s; p < s + n; p++) {
    putch(*p);
  }
  putch('\n');
#endif

  return n;
}

int printf(const char *fmt, ...) {
  if (fmt == 0) panic("null fmt");

  char buf[BUF_SIZE];
  memset(buf, 0, BUF_SIZE);

  va_list ap;
  va_start(ap, fmt);
  int off = vsnprintf(buf, BUF_SIZE, fmt, ap);
  va_end(ap);

  for (size_t i = 0; i < off; i++) {
    putch(buf[i]);
  }

  return off;
}

int vsprintf(char *buf, const char *fmt, va_list ap) { return vsnprintf(buf, SIZE_MAX, fmt, ap); }

int sprintf(char *out, const char *fmt, ...) {
  if (fmt == 0) panic("null fmt");

  va_list ap;
  va_start(ap, fmt);
  int off = vsnprintf(out, SIZE_MAX, fmt, ap);
  va_end(ap);

  return off;
}

int snprintf(char *buf, size_t sz, const char *fmt, ...) {
  if (fmt == 0) panic("null fmt");

  va_list ap;
  va_start(ap, fmt);
  int off = vsnprintf(buf, sz, fmt, ap);
  va_end(ap);

  return off;
}

int vsnprintf(char *buf, size_t sz, const char *fmt, va_list ap) {
  if (fmt == 0) panic("null fmt");

  char *s;
  size_t off = 0;

  for (size_t i = 0; off < sz && fmt[i] != '\0'; i++) {
    char c = fmt[i];
    if (c != '%') {
      off += sputc(buf + off, c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0) break;
    switch (c) {
      case 'd':
        off += sprintint(buf + off, va_arg(ap, int), 10, 1);
        break;
      case 'c':
        off += sputc(buf + off, va_arg(ap, int));
        break;
      case 'x':
        off += sprintint(buf + off, va_arg(ap, int), 16, 1);
        break;
      case 's':
        if ((s = va_arg(ap, char *)) == 0) {
          s = "(null)";
        }
        for (; *s && off < sz; s++) {
          off += sputc(buf + off, *s);
        }
        break;
      case '%':
        off += sputc(buf + off, '%');
        break;
      default:
        // Print unknown % sequence to draw attention.
        off += sputc(buf + off, '%');
        off += sputc(buf + off, c);
        break;
    }
  }
  buf[off] = '\0';

#if PUTCH_DEBUG
  for (char *p = buf; p < buf + off; p++) {
    putch(*p);
  }
  putch('\n');
#endif

  return off;
}

#endif
