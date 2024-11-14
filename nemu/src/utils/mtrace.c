#include "common.h"

#ifdef CONFIG_MTRACE

#define MTRACE_BUF_SIZE 256

static struct {
  vaddr_t addr;
  vaddr_t pc;
} buf[MTRACE_BUF_SIZE];

int rear = 0;

void mtrace_log(vaddr_t addr, vaddr_t pc) {
  buf[rear].addr = addr;
  buf[rear].pc = pc;
  rear = (rear + 1) % MTRACE_BUF_SIZE;
}

void mtrace_dump() {
  for (int i = 0; i < MTRACE_BUF_SIZE; i++) {
    if (buf[i].pc != 0) {
      printf("%x: %x\n", buf[i].pc, buf[i].addr);
    }
  }
}

#endif