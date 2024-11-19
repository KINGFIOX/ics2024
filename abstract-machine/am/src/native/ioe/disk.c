#include <am.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BLKSZ 512

static int disk_size = 0;
static FILE *fp = NULL;

void __am_disk_init() {
  const char *diskimg = getenv("diskimg");  // not set in Makefile
  if (diskimg) {
    fp = fopen(diskimg, "r+");
    if (fp) {
      fseek(fp, 0, SEEK_END);
      disk_size = (ftell(fp) + 511) / 512;
      rewind(fp);
    }
  }
}

void __am_disk_config(AM_DISK_CONFIG_T *cfg) {
  cfg->present = (fp != NULL);  // bool, if the fp has been set
  cfg->blksz = BLKSZ;
  cfg->blkcnt = disk_size;
}

void __am_disk_status(AM_DISK_STATUS_T *stat) { stat->ready = 1; }

void __am_disk_blkio(AM_DISK_BLKIO_T *io) {
  if (fp) {
    fseek(fp, io->blkno * BLKSZ, SEEK_SET);
    int ret;
    if (io->write)
      // NOTE: size_t fwrite(const void ptr[restrict .size * .nmemb], size_t size, size_t nmemb, FILE *restrict stream);
      // - size: 每个元素的大小
      // - nmemb: 要写入的元素的个数
      ret = fwrite(io->buf, io->blkcnt * BLKSZ, 1, fp);
    else
      ret = fread(io->buf, io->blkcnt * BLKSZ, 1, fp);
    assert(ret == 1);
  }
}
