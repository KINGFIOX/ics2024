# answer

##

hello.o 去掉符号表以后, 就不能链接了.

## case

### slider

![](image/slider1.png)

![](image/slider2.png)

### demo

#### ant

![](image/ant.png)

#### galton

![](image/galton.png)

#### hanoi

![](image/hanoi.png)

#### game-of-life

![](image/game-of-life.png)

#### aclock

![](image/aclock.png)

#### amatrix

![](image/amatrix.png)

#### donut

![](image/donut.png)

#### bf

![](image/bf.png)

### type

![](image/type.png)

### snake

![](image/snake.png)

### bad apple

![](image/bad-apple.png)

### mario

![](image/mario.png)

## 程序是个状态机

程序是个状态机 理解 YEMU 的执行过程, 具体请参考 [这里](https://nju-projectn.github.io/ics-pa-gitbook/ics2024/2.1.html#%E7%90%86%E8%A7%A3yemu%E5%A6%82%E4%BD%95%E6%89%A7%E8%A1%8C%E7%A8%8B%E5%BA%8F).

![](image/yemu.png)

状态包括: Reg, Mem, PC

## 一条指令在 NEMU 中的执行过程

RTFSC 请整理一条指令在 NEMU 中的执行过程, 具体请参考 [这里](https://nju-projectn.github.io/ics-pa-gitbook/ics2024/2.2.html#rtfsc%E7%90%86%E8%A7%A3%E6%8C%87%E4%BB%A4%E6%89%A7%E8%A1%8C%E7%9A%84%E8%BF%87%E7%A8%8B).

![](image/exec.png)

## 程序如何运行

程序如何运行 理解打字小游戏如何运行, 具体请参考 [这里](https://nju-projectn.github.io/ics-pa-gitbook/ics2024/2.5.html#%E6%B8%B8%E6%88%8F%E6%98%AF%E5%A6%82%E4%BD%95%E8%BF%90%E8%A1%8C%E7%9A%84).

按下 keyboard, SDL_PollEvent -> send_key -> key_enqueue.

io_read(AM_INPUT_KEYBOARD)
-> `AM_INPUT_KEYBRD_T ev = ({ AM_INPUT_KEYBRD_T __io_param; ioe_read(AM_INPUT_KEYBRD, &__io_param); __io_param; });`
-> `ioe_read(int reg, void *buf)` -> `((handler_t)lut[reg])(buf)` 这里是 AM_INPUT_KEYBRD `void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd)`
-> `uint32_t kc = inl(KBD_ADDR);`

-> (decode) `static inline word_t in_(int w, word_t port)` -> `uint32_t pio_read(ioaddr_t addr, int len);`
-> `word_t data = map_read(addr, len, map);` -> `invoke_callback(map->callback, offset, len, false);`
-> `static void i8042_data_io_handler(uint32_t offset, int len, bool is_write)`
-> `i8042_data_port_base[0] = key_dequeue();`

```c
while (1) {
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    // ...
}
```

## 编译与链接

编译与链接 在 nemu/include/cpu/ifetch.h 中, 你会看到由 static inline 开头定义的 inst_fetch()函数.
分别尝试去掉 static, 去掉 inline 或去掉两者, 然后重新进行编译, 你可能会看到发生错误.
请分别解释为什么这些错误会发生/不发生? 你有办法证明你的想法吗?
