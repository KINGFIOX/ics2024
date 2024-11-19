#include "common.h"

#ifdef CONFIG_MTRACE

#define MTRACE_BUF_SIZE 256

enum RW_TYPE { NONE = 0, READ, WRITE };

static struct {
  vaddr_t addr;
  vaddr_t pc;
  enum RW_TYPE type;
  word_t data;
} buf[MTRACE_BUF_SIZE];

int rear = 0;

void mtrace_log(vaddr_t addr, vaddr_t pc, word_t data, bool rw) {
  buf[rear].addr = addr;
  buf[rear].pc = pc;
  buf[rear].type = rw ? WRITE : READ;
  buf[rear].data = data;
  rear = (rear + 1) % MTRACE_BUF_SIZE;
}

void mtrace_dump(void) {
  for (int i = rear + 1; i != rear; i = (i + 1) % MTRACE_BUF_SIZE) {
    if (buf[i].type == NONE) {
      continue;
    }
    const char* str = buf[i].type == READ ? "READ " : "WRITE";
    if (buf[i].pc != 0) {
      printf("0x%08x: %s 0x%08x : ", buf[i].pc, str, buf[i].addr);
      printf("%02x %02x %02x %02x\n", buf[i].data & 0xff, (buf[i].data >> 8) & 0xff, (buf[i].data >> 16) & 0xff, (buf[i].data >> 24) & 0xff);
    }
  }
}

#endif
