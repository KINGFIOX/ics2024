#include <common.h>

#ifdef CONFIG_FTRACE

char *elf_file = NULL;

__attribute__((unused)) static long load_elf(void) {
  if (elf_file == NULL) {
    Log("No elf is given. Use the default build-in elf.");
    return 4096;  // built-in elf size
  }

  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can not open '%s'", elf_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The elf is %s, size = %ld", elf_file, size);

  fseek(fp, 0, SEEK_SET);

  fclose(fp);
  return size;
}

#endif
