#include <am.h>
#include <klib.h>
#include <nemu.h>

#include "klib-macros.h"

#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)       // 0x0000_0200
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)   // 0x0000_0204
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)    // 0x0000_0208
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)  // 0x0000_020c
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)       // 0x0000_0210
#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)      // 0x0000_0214
#define AUDIO_NR_WRITE_ADDR (AUDIO_ADDR + 0x18)   // 0x0000_0214

/* ---------- audio ctrl ---------- */

void __am_audio_init() {
  AM_AUDIO_CONFIG_T cfg __attribute__((unused)) = io_read(AM_AUDIO_CONFIG);
  // do nothing
}

/* ---------- audio ctrl ---------- */

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);  // init should be the last
}

/* ---------- audio ctrl ---------- */

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
  //
}

// cpu 中的是: sbuf_front.(消费者)
// os 中的是: sbuf_rear.(生产者)
static int ab_rear = 0;

static int write_once(uint8_t *buf, int len) {
  assert(len >= 0);
  assert(buf != NULL);
  const int ab_cnt = inl(AUDIO_COUNT_ADDR);
  const int ab_size = inl(AUDIO_SBUF_SIZE_ADDR) - ab_cnt;
  uint8_t *const ab = (uint8_t *)AUDIO_SBUF_ADDR;  // mmio
  int n = len < ab_size - 1 ? len : ab_size - 1;   // min(len, ab_size - 1)
  for (int i = 0; i < n; i++) {
    ab[(ab_rear + i) % ab_size] = buf[i];
  }
  outl(AUDIO_NR_WRITE_ADDR, n);
  return n;
}

static void audio_write(const uint8_t *buf, int len) {
  int nwrite = 0;
  while (nwrite < len) {
    int n = write_once((uint8_t *)buf + nwrite, len - nwrite);
    nwrite += n;
  }
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  int len = ctl->buf.end - ctl->buf.start;
  audio_write(ctl->buf.start, len);
}

/* ---------- audio config ---------- */

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  int ab_size = inl(AUDIO_SBUF_SIZE_ADDR);
  (*cfg) = (AM_AUDIO_CONFIG_T){.present = true, .bufsize = ab_size};
}