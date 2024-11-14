#include "common.h"

#ifdef CONFIG_MTRACE

static struct {
  vaddr_t addr;
  vaddr_t pc;
} *buf = NULL;

static size_t buf_size = 0;
static size_t buf_len = 0;

#define MTRACE_BUF_SIZE 256

void mtrace_log(vaddr_t addr, vaddr_t pc) {
  if (buf_len + 8 > buf_size) {
    if (buf == NULL) {
      buf = malloc(MTRACE_BUF_SIZE);
      assert(buf);
      buf_size = MTRACE_BUF_SIZE;
    } else {
      buf = realloc(buf, buf_size * 2);
      assert(buf);
      buf_size *= 2;
    }
  }
  buf[buf_len].addr = addr;
  buf[buf_len].pc = pc;
  buf_len += 1;
}

void mtrace_dump() {
  for (size_t i = 0; i < buf_len; i++) {
    printf("%x: %x\n", buf[i].pc, buf[i].addr);
  }
}

#endif
