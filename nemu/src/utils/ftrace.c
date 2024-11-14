#include <common.h>
#include <debug.h>
#include <fcntl.h>
#include <libelf.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef CONFIG_FTRACE

#define HISTORY_SIZE 1024
#define MAX_DEPTH 128

static int elf_fd = -1;

enum FUNC_TYPE { CALL, RET };

static struct {
  vaddr_t pc;
  int depth;
  enum FUNC_TYPE type;
} call_history[HISTORY_SIZE];
static int len = 0;

static int depth = 0;
static vaddr_t call_stack[MAX_DEPTH];

void push_call_stack(vaddr_t pc) {
  Assert(depth < HISTORY_SIZE, "call stack overflow");
  call_history[len].type = CALL;
  call_history[len].pc = pc;
  call_history[len].depth = depth;
  len++;
  call_stack[depth++] = pc;
}

void pop_call_stack(void) {
  Assert(depth > 0, "call stack underflow");
  vaddr_t pc = call_stack[depth];
  call_history[len].type = RET;
  call_history[len].pc = pc;
  call_history[len].depth = depth;
  len++;
  call_stack[depth] = 0;
  depth--;
}

static void close_elf(void) {
  Assert(elf_fd != -1, "elf_fd is NULL");
  close(elf_fd);
  elf_fd = -1;
}

void load_elf(const char *elf_file) {
  Assert(elf_file, "No elf is given. Use the default build-in elf.");
  elf_fd = open(elf_file, O_RDONLY);
  Assert(elf_fd, "Can not open '%s'", elf_file);

  atexit(&close_elf);
}

void call_stack_dump(void) {
  Assert(elf_fd, "elf_fd is NULL");
  Elf *elf = elf_begin(elf_fd, ELF_C_READ, NULL);
  Assert(elf, "failed to open elf file");

  elf_end(elf);
}

#endif
