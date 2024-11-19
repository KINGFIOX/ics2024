#include <common.h>
#include <debug.h>
#include <elf.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef CONFIG_DTRACE

#define DTRACE_HISTORY_SIZE 256

enum DevType { T_NONE = 0, T_PIO, T_MMIO };

static struct {
  vaddr_t pc;
  const char* name;
  paddr_t addr;  // only if mmio
  int len;       // only if mmio
  enum DevType type;
  bool is_write;
  word_t data;
} buf[DTRACE_HISTORY_SIZE];

static int rear = 0;

void dtrace_log(vaddr_t pc, word_t data, const char* name, bool is_pio, paddr_t addr, int len, bool is_write) {
  buf[rear].pc = pc;
  buf[rear].data = data;
  buf[rear].name = name;
  if (is_pio) {
    buf[rear].type = T_PIO;
  } else {
    buf[rear].type = T_MMIO;
  }
  buf[rear].addr = addr;
  buf[rear].len = len;
  buf[rear].is_write = is_write;
  rear = (rear + 1) % DTRACE_HISTORY_SIZE;
}

void dtrace_dump(void) {
  for (int i = rear + 1; i != rear; i = (i + 1) % DTRACE_HISTORY_SIZE) {
    if (buf[i].type == T_NONE) {
      continue;
    }
    const char* str = buf[i].is_write ? "WRITE" : "READ ";
    if (buf[i].pc != 0) {
      printf("0x%08x: %s 0x%08x(%s) : ", buf[i].pc, str, buf[i].addr, buf[i].name);
      printf("%02x %02x %02x %02x\n", buf[i].data & 0xff, (buf[i].data >> 8) & 0xff, (buf[i].data >> 16) & 0xff, (buf[i].data >> 24) & 0xff);
    }
  }
}

#endif
