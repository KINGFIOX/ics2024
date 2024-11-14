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

void free(void *ap) {
  Header *bp = (Header *)ap - 1;
  Header *prevp = &dummy;
  Header *p;
  for (p = prevp->s.next; p != NULL; prevp = p, p = p->s.next) {  // find the pos to insert, 每次插入链表的时候都是有序的
    if (prevp <= bp && bp < p) {
      break;
    }
  }
  if (p == NULL) {  // rear
    // dummy
  }
}

void *malloc(size_t nbytes) {
  const uint32_t nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;  // 向上对齐 + 1. 都是按照一个 Header 来对齐的

  if (dummy.s.next == NULL) {  // initialize the free_list and the dummy node
    dummy.s.next = heap.start;
    dummy.s.size = 0;
    dummy.s.next->s.next = NULL;
    dummy.s.next->s.size = (uint32_t)((char *)heap.end - (char *)heap.start) / sizeof(Header);
  }

  Header *prevp = &dummy;

  for (Header *p = prevp->s.next; p != NULL; prevp = p, p = p->s.next) {
    if (p->s.size >= nunits) {
      Header *ret;
      if (p->s.size == nunits) {
        ret = p;
        prevp->s.next = p->s.next;  // delete this node from free_list(linked list)
      } else {
        ret = p + (p->s.size - nunits);
        ret->s.size = nunits;
        p->s.size -= nunits;
      }
      printf("ret: %x\n", ret);
      return (void *)(ret + 1);  // skip the header
    }
  }

  return NULL;
}

#else

void *malloc(size_t size) { return NULL; }

#endif

#endif
