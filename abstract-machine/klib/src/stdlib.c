#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) { next = seed; }

int abs(int x) { return (x < 0 ? -x : x); }

int atoi(const char *nptr) {
  int x = 0;
  while (*nptr == ' ') {
    nptr++;
  }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr++;
  }
  return x;
}

// On native, malloc() will be called during initializaion of C runtime.
// Therefore do not call panic() here, else it will yield a dead recursion:
//   panic() -> putchar() -> (glibc) -> malloc() -> panic()

#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))

extern Area heap;

// 这里比较有意思的是: i386 下: ptr, long 都是 4byte

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint32_t size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;

static Header *freep;

void free(void *ap) {
  Header *bp = (Header *)ap - 1;
  Header *p;
  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) break;
  if (bp + bp->s.size == p->s.ptr) {
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if (p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else {
    p->s.ptr = bp;
  }
  freep = p;
}

void *malloc(size_t nbytes) {
  const uint32_t nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;  // 向上对齐 + 1. 都是按照一个 Header 来对齐的

  Header *prevp = freep;
  if (prevp == NULL) {
    base.s.ptr = freep = prevp = heap.start;
    base.s.size = (char *)heap.end - (char *)heap.start;
  }

  for (Header *p = prevp->s.ptr; /*dead loop*/; prevp = p, p = p->s.ptr) {
    if (p->s.size >= nunits) {
      if (p->s.size == nunits) {
        prevp->s.ptr = p->s.ptr;  // delete this node from free_list(linked list)
      } else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      return (void *)(p + 1);
    }
    printf("base: %x, p: %x, freep: %x\n", base.s.ptr, p, freep);
    if (p == freep) {
      return NULL;
    }
  }
}

#else

void *malloc(size_t size) { return NULL; }

#endif

#endif
