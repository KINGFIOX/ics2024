#define _GNU_SOURCE

#include <SDL.h>
#include <fcntl.h>
#include <klib.h>
#include <unistd.h>

static int rfd = -1, wfd = -1;
static volatile int count = 0;  // count of bytes in the pipe

int pipe2(int pipefd[2], int flags);

void __am_audio_init() {
  int fds[2];
  int ret = pipe2(fds, O_NONBLOCK);
  assert(ret == 0);
  rfd = fds[0];  // read
  wfd = fds[1];  // write
}

/**
 * @brief
 *
 * @param userdata
 * @param stream (return)
 * @param len
 */
static void audio_play(void *userdata, uint8_t *stream, int len) {
  int nread = len;
  if (count < len) nread = count;

  int b = 0;
  while (b < nread) {
    int n = read(rfd, stream, nread);
    if (n > 0) b += n;
  }

  count -= nread;
  if (len > nread) {
    memset(stream + nread, 0, len - nread);
  }
}

static void audio_write(const uint8_t *buf, int len) {
  int nwrite = 0;
  while (nwrite < len) {
    int n = write(wfd, buf, len);
    if (n == -1) n = 0;  // because of non-block
    count += n;
    nwrite += n;
  }
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  SDL_AudioSpec s = {
      .freq = ctrl->freq,
      .format = AUDIO_S16SYS,
      .channels = ctrl->channels,
      .samples = ctrl->samples,
      .callback = audio_play,
      .userdata = NULL,
  };

  count = 0;
  int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
  if (ret == 0) {
    SDL_OpenAudio(&s, NULL);
    SDL_PauseAudio(0);
  }
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) { stat->count = count; }

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  int len = ctl->buf.end - ctl->buf.start;
  audio_write(ctl->buf.start, len);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = fcntl(rfd, F_GETPIPE_SZ);
}
