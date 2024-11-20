#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int w = cfg.width, h = cfg.height;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (int i = 0; i < w * h; i++) fb[i] = 0;  // rubbish data
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t screen_wh = inl(VGACTL_ADDR);
  uint32_t h = screen_wh & 0xffff;
  uint32_t w = (screen_wh >> 16) & 0xffff;
  *cfg = (AM_GPU_CONFIG_T){.present = true, .has_accel = false, .width = w, .height = h, .vmemsz = 0};
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y;  // position
  int w = ctl->w, h = ctl->h;  // size
  if (!ctl->sync && (w == 0 || h == 0)) return;
  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;
  for (int i = y; i < y + h; i++) {
    for (int j = x; j < x + w; j++) {
      fb[screen_w * i + j] = pixels[w * (i - y) + (j - x)];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) { status->ready = true; }

void __am_gpu_memcpy(AM_GPU_MEMCPY_T *cpy) {
  // uint32_t dest = cpy->dest;
  // char *src = cpy->src;
  // int size = cpy->size;
  // // TODO: impl
}

void __am_gpu_render(AM_GPU_RENDER_T *ren) {
  // uint32_t root = ren->root;
  // // TODO: impl
}
