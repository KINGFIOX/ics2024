#include <common.h>
#include <stdio.h>

#ifdef CONFIG_FTRACE

static FILE *elf_fp = NULL;

static void close_elf(void) {
  if (elf_fp) {
    fclose(elf_fp);
    elf_fp = NULL;
  }
}

void load_elf(const char *elf_file) {
  if (elf_file == NULL) {
    Log("No elf is given. Use the default build-in elf.");
    return;
  }

  elf_fp = fopen(elf_file, "rb");
  Assert(elf_fp, "Can not open '%s'", elf_file);

  fseek(elf_fp, 0, SEEK_END);
  long size = ftell(elf_fp);
  Log("The elf is %s, size = %ld", elf_file, size);
  fseek(elf_fp, 0, SEEK_SET);

  atexit(&close_elf);
}

#endif
