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

extern Area heap;

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
  Header *p;
  Header *bp = (Header *)ap - 1;
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

// On native, malloc() will be called during initializaion of C runtime.
// Therefore do not call panic() here, else it will yield a dead recursion:
//   panic() -> putchar() -> (glibc) -> malloc() -> panic()

#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))

static Header *morecore(uint32_t nu) {
  if (nu < 4096) nu = 4096;
  char *p = (char *)heap.start;
  if (p == (char *)-1) return 0;
  Header *hp = (Header *)p;
  hp->s.size = nu;
  free((void *)(hp + 1));
  return freep;
}

void *malloc(size_t nbytes) {
  Header *prevp;
  const uint32_t nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  if ((prevp = freep) == 0) {
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for (Header *p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
    if (p->s.size >= nunits) {
      if (p->s.size == nunits) {
        prevp->s.ptr = p->s.ptr;
      } else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      return (void *)(p + 1);
    }
    if (p == freep) {
      if ((p = morecore(nunits)) == 0) {
        return 0;
      }
    }
  }
}

#else

void *malloc(size_t size) { return NULL; }

#endif

#endif
