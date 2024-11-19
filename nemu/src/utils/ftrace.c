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

#ifdef CONFIG_FTRACE

#define FTRACE_HISTORY_SIZE 256
#define MAX_DEPTH 128

static int elf_fd = -1;

enum FUNC_TYPE { NONE = 0, CALL, RET };

static struct {
  vaddr_t pos;
  vaddr_t pc;
  int depth;
  enum FUNC_TYPE type;
} call_history[FTRACE_HISTORY_SIZE];
static int rear = 0;

static int depth = 0;
static vaddr_t call_stack[MAX_DEPTH];

void push_call_stack(vaddr_t pc, vaddr_t pos) {
  Assert(depth < FTRACE_HISTORY_SIZE, "call stack overflow");
  // printf("len = %d\n", len);
  call_history[rear].pos = pos;
  call_history[rear].type = CALL;
  call_history[rear].pc = pc;
  call_history[rear].depth = depth;
  rear = (rear + 1) % FTRACE_HISTORY_SIZE;
  call_stack[depth++] = pc;
}

void pop_call_stack(vaddr_t pos) {
  Assert(depth > 0, "call stack underflow");
  depth--;
  vaddr_t pc = call_stack[depth];
  call_history[rear].pos = pos;
  call_history[rear].type = RET;
  call_history[rear].pc = pc;
  call_history[rear].depth = depth;
  rear = (rear + 1) % FTRACE_HISTORY_SIZE;
  call_stack[depth] = 0;
}

static void close_elf(void) {
  Assert(elf_fd != -1, "elf_fd is NULL");
  close(elf_fd);
  elf_fd = -1;
}

void load_elf(const char *elf_file) {
  Assert(elf_file, "No elf is given. Use the default build-in elf.");
  elf_fd = open(elf_file, O_RDONLY);
  Assert(elf_fd != -1, "Can not open '%s'", elf_file);

  atexit(&close_elf);
}

static const char *func_name(Elf *elf, vaddr_t pc) {
  Elf_Scn *scn = NULL;
  while ((scn = elf_nextscn(elf, scn)) != NULL) {
    GElf_Shdr shdr;
    if (gelf_getshdr(scn, &shdr) != &shdr) {
      continue;
    }
    if (shdr.sh_type == SHT_SYMTAB) {
      Elf_Data *data = elf_getdata(scn, NULL);
      size_t sym_count = shdr.sh_size / shdr.sh_entsize;

      for (size_t i = 0; i < sym_count; ++i) {
        GElf_Sym sym;
        gelf_getsym(data, i, &sym);

        //
        if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC && sym.st_value == pc) {
          return elf_strptr(elf, shdr.sh_link, sym.st_name);
        }
      }
    }
  }
  return NULL;
}

static void dump_call_history(Elf *elf) {
  for (int i = rear + 1; i != rear; i = (i + 1) % FTRACE_HISTORY_SIZE) {
    if (call_history[i].type == NONE) {
      continue;
    }
    vaddr_t pos = call_history[i].pos;  // call point
    printf("0x%08x: ", pos);
    int depth = call_history[i].depth;  // indent
    for (int j = 0; j < depth; j++) {
      printf(" ");
    }
    vaddr_t pc = call_history[i].pc;
    if (pc == 0) {
      char *type;
      if (call_history[i].type == CALL) {
        type = "call";
      } else {
        type = "ret";
      }
      printf("%s pc = 0, i = %d, depth = %d\n", type, i, depth);
      continue;
    }
    const char *name = func_name(elf, pc);
    if (call_history[i].type == CALL) {
      printf("call [%s@%x]\n", name, pc);
    } else {
      printf("ret [%s@%x]\n", name, pc);
    }
  }
}

void call_stack_dump(void) {
  Assert(elf_version(EV_CURRENT) != EV_NONE, "libelf is out of date");  // must be called before elf_begin

  Assert(elf_fd, "elf_fd is NULL");
  Elf *elf = elf_begin(elf_fd, ELF_C_READ, NULL);
  Assert(elf != NULL, "failed to open elf file");

  dump_call_history(elf);

  elf_end(elf);
}

#endif
