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

- static 没有问题, 因为是 inline 了, 会把他当做是一个弱符号来用
- 去掉 inline 没有问题, 头文件会被拷贝到各自的 .c 中, 一个 .c 会被翻译成一个 .o, 然后 static 只是内部符号, 不会暴露到外面, 不会有问题
- 去掉 inline, static 有问题, 出现了多重定义

## 编译与链接

1. 在 nemu/include/common.h 中添加一行 volatile static int dummy; 然后重新编译 NEMU.
   请问重新编译后的 NEMU 含有多少个 dummy 变量的实体? 你是如何得到这个结果的?

2. 添加上题中的代码后, 再在 nemu/include/debug.h 中添加一行 volatile static int dummy;
   然后重新编译 NEMU. 请问此时的 NEMU 含有多少个 dummy 变量的实体? 与上题中 dummy 变量实体数目进行比较, 并解释本题的结果.

3. 修改添加的代码, 为两处 dummy 变量进行初始化:volatile static int dummy = 0; 然后重新编译 NEMU. 你发现了什么问题?
   为什么之前没有出现这样的问题? (回答完本题后可以删除添加的代码.)

## Makefile

了解 Makefile 请描述你在 am-kernels/kernels/hello/目录下敲入 make ARCH=$ISA-nemu 后, make程序如何组织.c和.h文件, 
最终生成可执行文件am-kernels/kernels/hello/build/hello-$ISA-nemu.elf.
(这个问题包括两个方面:Makefile 的工作方式和编译链接的过程.) 关于 Makefile 工作方式的提示:
Makefile 中使用了变量, 包含文件等特性.
Makefile 运用并重写了一些 implicit rules.
在 man make 中搜索-n 选项, 也许会对你有帮助.
