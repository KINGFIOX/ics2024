/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <SDL2/SDL.h>
#include <common.h>
#include <device/map.h>

// 在 enum 最后一个位置放上: nr_reg, 这种写法有意思
enum { reg_freq, reg_channels, reg_samples, reg_sbuf_size, reg_init, reg_front, reg_count, nr_reg /*number of regs*/ };

static uint8_t *sbuf = NULL;  // ring buffer
static uint32_t *audio_base = NULL;

static SDL_AudioSpec spec = {};

static int read_(uint8_t *stream, int len) {
  Assert(sbuf, "sbuf is not initialized");
  assert(0);
  const int sbuf_size = audio_base[reg_sbuf_size];
  const int front = audio_base[reg_front];
  for (int i = 0; i < len; i++) {
    stream[i] = sbuf[(front + i) % sbuf_size];
  }
  audio_base[reg_front] = (front + len) % sbuf_size;
  return len;
}

/**
 * @brief
 *
 * @param udata
 * @param stream
 * @param len 需要这么多数据
 */
static void sdl_audio_callback(void *udata, uint8_t *stream, int len) {
  const int count = audio_base[reg_count];

  int nread = count < len ? count : len;  // min

  int b = 0;
  while (b < nread) {
    int n = read_(stream + b, nread - b);
    b += n;
  }

  audio_base[reg_count] = count - nread;
  if (len > nread) {
    memset(stream + nread, 0, len - nread);
  }
}

static void audio_pio_handler(uint32_t offset, int len, bool is_write) {
  if (audio_base[reg_init] != 0) {
    spec.format = AUDIO_S16SYS;
    spec.userdata = NULL;
    spec.freq = audio_base[reg_freq];
    spec.channels = audio_base[reg_channels];
    spec.samples = audio_base[reg_samples];
    spec.callback = sdl_audio_callback;
    int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (ret == 0) {
      SDL_OpenAudio(&spec, NULL);  // 当调用 SDL_OpenAudio 后, SDL 开始音频播放. 当 SDL 库内部的 数据 buffer 低于一定阈值时, invoke callback
      SDL_PauseAudio(0);
    }
    audio_base[reg_init] = 0;
  }
}

static void audio_mmio_handler(uint32_t offset, int len, bool is_write) {
  assert(0);
  printf("sbuf[%d] = %d\n", offset, sbuf[offset]);
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_pio_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);  // sbuf_size
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, audio_mmio_handler);
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
}
