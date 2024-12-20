#include <SDL.h>
#include <am.h>

#define KEYDOWN_MASK 0x8000

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN] = {};
static int key_f /*front*/ = 0, key_r /*rear*/ = 0;  // in xx, (xx) 的时候, 环形缓冲区 pop

static SDL_mutex *key_queue_lock = NULL;

#define XX(k) [SDL_SCANCODE_##k] = AM_KEY_##k,

// #define AM_KEYS(_)
//   _(ESCAPE)
//   _(F1)
//   _(F2)
//   _(F3)
//   _(F4)
//   _(F5)
//   _(F6)
//   _(F7)

// XX(ESCAPE)
// XX(F1)
// XX(F2)
// XX(F3)
// XX(F4)
// XX(F5)
// XX(F6)
// XX(F7)

// [SDL_SCANCODE_ESCAPE] = AM_KEY_ESCAPE,
// [SDL_SCANCODE_F1] = AM_KEY_F1,
// [SDL_SCANCODE_F2] = AM_KEY_F2,
// [SDL_SCANCODE_F3] = AM_KEY_F3,
// [SDL_SCANCODE_F4] = AM_KEY_F4,
// [SDL_SCANCODE_F5] = AM_KEY_F5,
// [SDL_SCANCODE_F6] = AM_KEY_F6,
// [SDL_SCANCODE_F7] = AM_KEY_F7,

static int keymap[256] = {AM_KEYS(XX)};

// static int keymap[256] = {
//     [SDL_SCANCODE_ESCAPE] = AM_KEY_ESCAPE,
//     [SDL_SCANCODE_F1] = AM_KEY_F1,
//     [SDL_SCANCODE_F2] = AM_KEY_F2,
//     [SDL_SCANCODE_F3] = AM_KEY_F3,
//     [SDL_SCANCODE_F4] = AM_KEY_F4,
//     [SDL_SCANCODE_F5] = AM_KEY_F5,
//     [SDL_SCANCODE_F6] = AM_KEY_F6,
//     [SDL_SCANCODE_F7] = AM_KEY_F7,
//     [SDL_SCANCODE_F8] = AM_KEY_F8,
// };

// #define AM_KEY_NAMES(key) AM_KEY_##key,
// enum { AM_KEY_NONE = 0, AM_KEYS(AM_KEY_NAMES) };

static int event_thread(void *args) {
  SDL_Event event;
  while (1) {
    SDL_WaitEvent(&event);
    switch (event.type) {
      case SDL_QUIT:
        halt(0);         // trm
      case SDL_KEYDOWN:  // this keydown/keyup means: the key is pressed/released
      case SDL_KEYUP: {
        SDL_Keysym k = event.key.keysym;  // key that pressed or released
        int keydown = event.key.type == SDL_KEYDOWN;
        int scancode = k.scancode;
        if (keymap[scancode] != AM_KEY_NONE) {
          int am_code = keymap[scancode] | (keydown ? KEYDOWN_MASK : 0);
          SDL_LockMutex(key_queue_lock);
          key_queue[key_r] = am_code;
          key_r = (key_r + 1) % KEY_QUEUE_LEN;
          SDL_UnlockMutex(key_queue_lock);
          void __am_send_kbd_intr();
          __am_send_kbd_intr();  // signal to emulate the interrupt
        }
        break;
      }
    }
  }
}

void __am_input_init() {
  key_queue_lock = SDL_CreateMutex();
  SDL_CreateThread(event_thread, "event thread", NULL);
}

void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true; }

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  int k = AM_KEY_NONE;

  SDL_LockMutex(key_queue_lock);  // pop the key from the queue
  if (key_f != key_r) {
    k = key_queue[key_f];
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
  }
  SDL_UnlockMutex(key_queue_lock);

  kbd->keydown = (k & KEYDOWN_MASK ? true : false);
  kbd->keycode = k & ~KEYDOWN_MASK;
}
