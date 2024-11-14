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
  printf("elf file opened: %s\n", elf_file);
  Assert(elf_file, "No elf is given. Use the default build-in elf.");
  elf_fd = open(elf_file, O_RDONLY);
  Assert(elf_fd != -1, "Can not open '%s'", elf_file);

  atexit(&close_elf);
}

void call_stack_dump(void) {
  Assert(elf_version(EV_CURRENT) != EV_NONE, "libelf is out of date");

  Assert(elf_fd, "elf_fd is NULL");
  Elf *elf = elf_begin(elf_fd, ELF_C_READ, NULL);
  Assert(elf != NULL, "failed to open elf file");

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
        if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC && sym.st_value == call_history[0].pc) {
          printf("function found: %s\n", elf_strptr(elf, shdr.sh_link, sym.st_name));
        }
      }
    }
  }

  elf_end(elf);
}

#endif
