#include <am.h>
#include <klib.h>
#include <nemu.h>

#include "klib-macros.h"

static int buf_pos = 0;

void __am_audio_init() {
  AM_AUDIO_CONFIG_T cfg __attribute__((unused)) = io_read(AM_AUDIO_CONFIG);
  // do nothing
}

/* ---------- audio config ---------- */

#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)  // 0x0000_020c

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  int bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
  (*cfg) = (AM_AUDIO_CONFIG_T){.present = false, .bufsize = bufsize};
}

/* ---------- audio ctrl ---------- */

#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)      // 0x0000_0200
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)  // 0x0000_0204
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)   // 0x0000_0207
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)      // 0x0000_0210

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);
}

/* ---------- audio ctrl ---------- */

#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)  // 0x0000_0214

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
  //
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  const uint32_t buf_size = inl(AUDIO_SBUF_SIZE_ADDR) / sizeof(uint8_t);

  uint8_t *audio = (ctl->buf).start;
  uint32_t len = ((ctl->buf).end - (ctl->buf).start) / sizeof(uint8_t);

  while (len > buf_size - inl(AUDIO_COUNT_ADDR));  // blocking

  uint8_t *ab = (uint8_t *)(uintptr_t)AUDIO_ADDR;
  for (int i = 0; i < len; i++) {
    ab[buf_pos] = audio[i];
    buf_pos = (buf_pos + 1) % buf_size;
  }

  outl(AUDIO_COUNT_ADDR, inl(AUDIO_COUNT_ADDR) + len);
}