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
    union header *next;
    uint32_t size;
  } s;
  Align x;
};

typedef union header Header;

static Header dummy;

static Header *freep;  // point to the first header of free_list

void free(void *ap) {
  Header *bp = (Header *)ap - 1;
  Header *p;
  for (p = freep; !(bp > p && bp < p->s.next); p = p->s.next)
    if (p >= p->s.next && (bp > p || bp < p->s.next)) break;
  if (bp + bp->s.size == p->s.next) {
    bp->s.size += p->s.next->s.size;
    bp->s.next = p->s.next->s.next;
  } else
    bp->s.next = p->s.next;
  if (p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.next = bp->s.next;
  } else {
    p->s.next = bp;
  }
  freep = p;
}

void *malloc(size_t nbytes) {
  const uint32_t nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;  // 向上对齐 + 1. 都是按照一个 Header 来对齐的

  if (freep == NULL) {  // initialize the free_list and the dummy node
    dummy.s.next = heap.start;
    dummy.s.size = 0;
    freep = heap.start;
    freep->s.size = (char *)heap.end - (char *)heap.start;
    freep->s.next = NULL;
    printf("base: %x, base.s.ptr: %x, freep: %x, size: %x\n", &dummy, dummy.s.next, freep, freep->s.size);
  }
  Header *prevp = freep;

  for (Header *p = prevp->s.next; p != NULL; prevp = p, p = p->s.next) {
    if (p->s.size >= nunits) {
      if (p->s.size == nunits) {
        prevp->s.next = p->s.next;
      } else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
    }
    printf("base: %x, p: %x\n", dummy.s.next, (char *)p + 1);
    return (void *)(p + 1);
  }
  // for (Header *p = prevp->s.next; p != NULL; prevp = p, p = p->s.next) {
  //   if (p->s.size >= nunits) {
  //     if (p->s.size == nunits) {
  //       prevp->s.next = p->s.next;  // delete this node from free_list(linked list)
  //     } else {
  //       p->s.size -= nunits;
  //       p += p->s.size;
  //       p->s.size = nunits;
  //     }
  //     printf("base: %x, p: %x, freep: %x\n", dummy.s.next, (char *)p + 1, freep);
  //     freep = prevp;
  //     return (void *)(p + 1);
  //   }
  // }
  return NULL;
}

#else

void *malloc(size_t size) { return NULL; }

#endif

#endif
