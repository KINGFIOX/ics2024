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

0, 都是声明

`nm xxx.elf | grep "dummy"`

2. 添加上题中的代码后, 再在 nemu/include/debug.h 中添加一行 volatile static int dummy;
   然后重新编译 NEMU. 请问此时的 NEMU 含有多少个 dummy 变量的实体? 与上题中 dummy 变量实体数目进行比较, 并解释本题的结果.

0, 都是声明.

3. 修改添加的代码, 为两处 dummy 变量进行初始化:volatile static int dummy = 0; 然后重新编译 NEMU. 你发现了什么问题?
   为什么之前没有出现这样的问题? (回答完本题后可以删除添加的代码.)

链接错误, 重定义了. 之前没有重定义

## Makefile

了解 Makefile 请描述你在 am-kernels/kernels/hello/目录下敲入 make ARCH=$ISA-nemu 后, make程序如何组织.c和.h文件, 
最终生成可执行文件am-kernels/kernels/hello/build/hello-$ISA-nemu.elf.
(这个问题包括两个方面:Makefile 的工作方式和编译链接的过程.) 关于 Makefile 工作方式的提示:
Makefile 中使用了变量, 包含文件等特性.
Makefile 运用并重写了一些 implicit rules.
在 man make 中搜索-n 选项, 也许会对你有帮助.

`am-kernels/kernels/hello/Makefile`
-> `abstract-machine/Makefile`
---> `abstract-machine/scripts/x86-nemu.mk`
-----> `abstract-machine/scripts/isa/x86.mk`
-----> `abstract-machine/scripts/platform/nemu.mk`
--->`$(DST_DIR)/xxx.d`

`-n`: Print the commands that would be executed

```bash
wangfiox@localhost:am-kernels/kernels/hello> make run -n ARCH=x86-nemu
# Building hello-run [x86-nemu]
make -s -C abstract-machine/am archive
# Building am-archive [x86-nemu]
make -s -C abstract-machine/klib archive
# Building klib-archive [x86-nemu]
mkdir -p am-kernels/kernels/hello/build/x86-nemu/ && echo + CC hello.c
gcc -std=gnu11 -O0 -g -MMD -Wall -Werror -Iam-kernels/kernels/hello/include -Iabstract-machine/am/include/ -Iabstract-machine/klib/include/ -D__ISA__=\"x86\" -D__ISA_X86__ -D__ARCH__=x86-nemu -D__ARCH_X86_NEMU -D__PLATFORM__=nemu -D__PLATFORM_NEMU -DARCH_H=\"arch/x86-nemu.h\" -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector -Wno-main -U_FORTIFY_SOURCE -fvisibility=hidden -m32 -fno-pic -fno-omit-frame-pointer -march=i386 -fcf-protection=none  --param=min-pagesize=0  -fdata-sections -ffunction-sections -Iabstract-machine/am/src/platform/nemu/include -DMAINARGS_MAX_LEN=64 -DMAINARGS_PLACEHOLDER=\""The insert-arg rule in Makefile will insert mainargs here."\" -DISA_H=\"x86/x86.h\"  -c -o am-kernels/kernels/hello/build/x86-nemu/hello.o am-kernels/kernels/hello/hello.c
echo \# Creating image [x86-nemu]
echo + LD "->" build/hello-x86-nemu.elf
ld -z noexecstack -Tabstract-machine/scripts/linker.ld -melf_i386 --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0 --gc-sections -e _start --defsym=_pmem_start=0x0 --defsym=_entry_offset=0x100000 -o am-kernels/kernels/hello/build/hello-x86-nemu.elf --start-group am-kernels/kernels/hello/build/x86-nemu/hello.o abstract-machine/am/build/am-x86-nemu.a abstract-machine/klib/build/klib-x86-nemu.a --end-group
objdump -d am-kernels/kernels/hello/build/hello-x86-nemu.elf > am-kernels/kernels/hello/build/hello-x86-nemu.txt
echo + OBJCOPY "->" build/hello-x86-nemu.bin
objcopy -S --set-section-flags .bss=alloc,contents -O binary am-kernels/kernels/hello/build/hello-x86-nemu.elf am-kernels/kernels/hello/build/hello-x86-nemu.bin
python abstract-machine/tools/insert-arg.py am-kernels/kernels/hello/build/hello-x86-nemu.bin 64 "The insert-arg rule in Makefile will insert mainargs here." ""
make -C nemu ISA=x86 run ARGS="-l am-kernels/kernels/hello/build/nemu-log.txt" IMG=am-kernels/kernels/hello/build/hello-x86-nemu.bin ELF=am-kernels/kernels/hello/build/hello-x86-nemu.elf
make[1]: Entering directory 'nemu'
echo + CC src/device/device.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/device.o src/device/device.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/device.d  nemu/build/obj-x86-nemu-interpreter/src/device/device.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/device.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/device.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/device.d
echo + CC src/device/alarm.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/alarm.o src/device/alarm.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/alarm.d  nemu/build/obj-x86-nemu-interpreter/src/device/alarm.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/alarm.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/alarm.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/alarm.d
echo + CC src/device/intr.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/intr.o src/device/intr.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/intr.d  nemu/build/obj-x86-nemu-interpreter/src/device/intr.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/intr.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/intr.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/intr.d
echo + CC src/device/serial.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/serial.o src/device/serial.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/serial.d  nemu/build/obj-x86-nemu-interpreter/src/device/serial.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/serial.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/serial.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/serial.d
echo + CC src/device/timer.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/timer.o src/device/timer.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/timer.d  nemu/build/obj-x86-nemu-interpreter/src/device/timer.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/timer.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/timer.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/timer.d
echo + CC src/device/keyboard.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/keyboard.o src/device/keyboard.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/keyboard.d  nemu/build/obj-x86-nemu-interpreter/src/device/keyboard.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/keyboard.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/keyboard.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/keyboard.d
echo + CC src/device/vga.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/vga.o src/device/vga.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/vga.d  nemu/build/obj-x86-nemu-interpreter/src/device/vga.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/vga.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/vga.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/vga.d
echo + CC src/device/audio.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/audio.o src/device/audio.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/audio.d  nemu/build/obj-x86-nemu-interpreter/src/device/audio.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/audio.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/audio.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/audio.d
echo + CC src/device/disk.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/disk.o src/device/disk.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/disk.d  nemu/build/obj-x86-nemu-interpreter/src/device/disk.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/disk.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/disk.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/disk.d
echo + CC src/nemu-main.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/nemu-main.o src/nemu-main.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/nemu-main.d  nemu/build/obj-x86-nemu-interpreter/src/nemu-main.o unused >  nemu/build/obj-x86-nemu-interpreter/src/nemu-main.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/nemu-main.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/nemu-main.d
echo + CC src/device/io/mmio.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/io/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/io/mmio.o src/device/io/mmio.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/io/mmio.d  nemu/build/obj-x86-nemu-interpreter/src/device/io/mmio.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/io/mmio.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/io/mmio.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/io/mmio.d
echo + CC src/device/io/port-io.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/io/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/io/port-io.o src/device/io/port-io.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/io/port-io.d  nemu/build/obj-x86-nemu-interpreter/src/device/io/port-io.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/io/port-io.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/io/port-io.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/io/port-io.d
echo + CC src/device/io/map.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/device/io/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/device/io/map.o src/device/io/map.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/device/io/map.d  nemu/build/obj-x86-nemu-interpreter/src/device/io/map.o unused >  nemu/build/obj-x86-nemu-interpreter/src/device/io/map.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/device/io/map.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/device/io/map.d
echo + CC src/engine/interpreter/init.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/init.o src/engine/interpreter/init.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/init.d  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/init.o unused >  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/init.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/init.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/init.d
echo + CC src/engine/interpreter/hostcall.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/hostcall.o src/engine/interpreter/hostcall.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/hostcall.d  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/hostcall.o unused >  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/hostcall.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/hostcall.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/hostcall.d
echo + CC src/cpu/difftest/ref.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/ref.o src/cpu/difftest/ref.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/ref.d  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/ref.o unused >  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/ref.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/ref.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/ref.d
echo + CC src/cpu/difftest/dut.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/dut.o src/cpu/difftest/dut.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/dut.d  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/dut.o unused >  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/dut.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/dut.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/dut.d
echo + CC src/cpu/cpu-exec.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/cpu/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/cpu/cpu-exec.o src/cpu/cpu-exec.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/cpu/cpu-exec.d  nemu/build/obj-x86-nemu-interpreter/src/cpu/cpu-exec.o unused >  nemu/build/obj-x86-nemu-interpreter/src/cpu/cpu-exec.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/cpu/cpu-exec.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/cpu/cpu-exec.d
echo + CC src/monitor/sdb/expr.tab.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expr.tab.o src/monitor/sdb/expr.tab.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expr.tab.d  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expr.tab.o unused >  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expr.tab.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expr.tab.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expr.tab.d
echo + CC src/monitor/sdb/sdb.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/sdb.o src/monitor/sdb/sdb.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/sdb.d  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/sdb.o unused >  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/sdb.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/sdb.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/sdb.d
echo + CC src/monitor/sdb/watchpoint.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/watchpoint.o src/monitor/sdb/watchpoint.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/watchpoint.d  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/watchpoint.o unused >  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/watchpoint.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/watchpoint.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/watchpoint.d
echo + CC src/monitor/sdb/yyac.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/yyac.o src/monitor/sdb/yyac.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/yyac.d  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/yyac.o unused >  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/yyac.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/yyac.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/yyac.d
echo + CC src/monitor/sdb/expression.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expression.o src/monitor/sdb/expression.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expression.d  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expression.o unused >  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expression.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expression.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expression.d
echo + CC src/monitor/monitor.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/monitor/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/monitor/monitor.o src/monitor/monitor.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/monitor/monitor.d  nemu/build/obj-x86-nemu-interpreter/src/monitor/monitor.o unused >  nemu/build/obj-x86-nemu-interpreter/src/monitor/monitor.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/monitor/monitor.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/monitor/monitor.d
echo + CC src/utils/state.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/utils/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/utils/state.o src/utils/state.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/utils/state.d  nemu/build/obj-x86-nemu-interpreter/src/utils/state.o unused >  nemu/build/obj-x86-nemu-interpreter/src/utils/state.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/utils/state.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/utils/state.d
echo + CC src/utils/timer.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/utils/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/utils/timer.o src/utils/timer.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/utils/timer.d  nemu/build/obj-x86-nemu-interpreter/src/utils/timer.o unused >  nemu/build/obj-x86-nemu-interpreter/src/utils/timer.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/utils/timer.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/utils/timer.d
echo + CC src/utils/log.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/utils/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/utils/log.o src/utils/log.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/utils/log.d  nemu/build/obj-x86-nemu-interpreter/src/utils/log.o unused >  nemu/build/obj-x86-nemu-interpreter/src/utils/log.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/utils/log.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/utils/log.d
echo + CC src/utils/disasm.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/utils/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/utils/disasm.o src/utils/disasm.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/utils/disasm.d  nemu/build/obj-x86-nemu-interpreter/src/utils/disasm.o unused >  nemu/build/obj-x86-nemu-interpreter/src/utils/disasm.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/utils/disasm.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/utils/disasm.d
echo + CC src/utils/mtrace.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/utils/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/utils/mtrace.o src/utils/mtrace.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/utils/mtrace.d  nemu/build/obj-x86-nemu-interpreter/src/utils/mtrace.o unused >  nemu/build/obj-x86-nemu-interpreter/src/utils/mtrace.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/utils/mtrace.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/utils/mtrace.d
echo + CC src/utils/ftrace.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/utils/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/utils/ftrace.o src/utils/ftrace.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/utils/ftrace.d  nemu/build/obj-x86-nemu-interpreter/src/utils/ftrace.o unused >  nemu/build/obj-x86-nemu-interpreter/src/utils/ftrace.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/utils/ftrace.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/utils/ftrace.d
echo + CC src/utils/dtrace.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/utils/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/utils/dtrace.o src/utils/dtrace.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/utils/dtrace.d  nemu/build/obj-x86-nemu-interpreter/src/utils/dtrace.o unused >  nemu/build/obj-x86-nemu-interpreter/src/utils/dtrace.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/utils/dtrace.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/utils/dtrace.d
echo + CC src/memory/paddr.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/memory/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/memory/paddr.o src/memory/paddr.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/memory/paddr.d  nemu/build/obj-x86-nemu-interpreter/src/memory/paddr.o unused >  nemu/build/obj-x86-nemu-interpreter/src/memory/paddr.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/memory/paddr.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/memory/paddr.d
echo + CC src/memory/vaddr.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/memory/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/memory/vaddr.o src/memory/vaddr.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/memory/vaddr.d  nemu/build/obj-x86-nemu-interpreter/src/memory/vaddr.o unused >  nemu/build/obj-x86-nemu-interpreter/src/memory/vaddr.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/memory/vaddr.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/memory/vaddr.d
echo + CC src/isa/x86/difftest/dut.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/difftest/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/difftest/dut.o src/isa/x86/difftest/dut.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/difftest/dut.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/difftest/dut.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/difftest/dut.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/difftest/dut.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/difftest/dut.d
echo + CC src/isa/x86/system/intr.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/intr.o src/isa/x86/system/intr.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/intr.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/intr.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/intr.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/intr.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/intr.d
echo + CC src/isa/x86/system/mmu.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/mmu.o src/isa/x86/system/mmu.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/mmu.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/mmu.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/mmu.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/mmu.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/mmu.d
echo + CC src/isa/x86/logo.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/logo.o src/isa/x86/logo.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/logo.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/logo.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/logo.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/logo.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/logo.d
echo + CC src/isa/x86/init.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/init.o src/isa/x86/init.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/init.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/init.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/init.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/init.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/init.d
echo + CC src/isa/x86/inst/push.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/push.o src/isa/x86/inst/push.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/push.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/push.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/push.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/push.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/push.d
echo + CC src/isa/x86/inst/arithmetic.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/arithmetic.o src/isa/x86/inst/arithmetic.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/arithmetic.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/arithmetic.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/arithmetic.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/arithmetic.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/arithmetic.d
echo + CC src/isa/x86/inst/decode.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/decode.o src/isa/x86/inst/decode.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/decode.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/decode.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/decode.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/decode.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/decode.d
echo + CC src/isa/x86/inst/jmp.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/jmp.o src/isa/x86/inst/jmp.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/jmp.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/jmp.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/jmp.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/jmp.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/jmp.d
echo + CC src/isa/x86/reg.c
mkdir -p nemu/build/obj-x86-nemu-interpreter/src/isa/x86/
clang -O2 -MMD -Wall -Werror -Inemu/include -Inemu/src/engine/interpreter -Inemu/src/isa/x86/include -I tools/capstone/repo/include -fPIE -O0  -Og -ggdb3 -fsanitize=address -DITRACE_COND=true -D__GUEST_ISA__=x86 -c -o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/reg.o src/isa/x86/reg.c
nemu/tools/fixdep/build/fixdep  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/reg.d  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/reg.o unused >  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/reg.d.tmp
mv  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/reg.d.tmp  nemu/build/obj-x86-nemu-interpreter/src/isa/x86/reg.d
echo + LD nemu/build/x86-nemu-interpreter
clang++ -o nemu/build/x86-nemu-interpreter nemu/build/obj-x86-nemu-interpreter/src/device/device.o nemu/build/obj-x86-nemu-interpreter/src/device/alarm.o nemu/build/obj-x86-nemu-interpreter/src/device/intr.o nemu/build/obj-x86-nemu-interpreter/src/device/serial.o nemu/build/obj-x86-nemu-interpreter/src/device/timer.o nemu/build/obj-x86-nemu-interpreter/src/device/keyboard.o nemu/build/obj-x86-nemu-interpreter/src/device/vga.o nemu/build/obj-x86-nemu-interpreter/src/device/audio.o nemu/build/obj-x86-nemu-interpreter/src/device/disk.o nemu/build/obj-x86-nemu-interpreter/src/nemu-main.o nemu/build/obj-x86-nemu-interpreter/src/device/io/mmio.o nemu/build/obj-x86-nemu-interpreter/src/device/io/port-io.o nemu/build/obj-x86-nemu-interpreter/src/device/io/map.o nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/init.o nemu/build/obj-x86-nemu-interpreter/src/engine/interpreter/hostcall.o nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/ref.o nemu/build/obj-x86-nemu-interpreter/src/cpu/difftest/dut.o nemu/build/obj-x86-nemu-interpreter/src/cpu/cpu-exec.o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expr.tab.o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/sdb.o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/watchpoint.o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/yyac.o nemu/build/obj-x86-nemu-interpreter/src/monitor/sdb/expression.o nemu/build/obj-x86-nemu-interpreter/src/monitor/monitor.o nemu/build/obj-x86-nemu-interpreter/src/utils/state.o nemu/build/obj-x86-nemu-interpreter/src/utils/timer.o nemu/build/obj-x86-nemu-interpreter/src/utils/log.o nemu/build/obj-x86-nemu-interpreter/src/utils/disasm.o nemu/build/obj-x86-nemu-interpreter/src/utils/mtrace.o nemu/build/obj-x86-nemu-interpreter/src/utils/ftrace.o nemu/build/obj-x86-nemu-interpreter/src/utils/dtrace.o nemu/build/obj-x86-nemu-interpreter/src/memory/paddr.o nemu/build/obj-x86-nemu-interpreter/src/memory/vaddr.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/difftest/dut.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/intr.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/system/mmu.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/logo.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/init.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/push.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/arithmetic.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/decode.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/inst/jmp.o nemu/build/obj-x86-nemu-interpreter/src/isa/x86/reg.o  -O2 -O0  -Og -ggdb3 -fsanitize=address  -L/usr/lib64 -lSDL2 -lreadline -ldl -pie -lelf
git add nemu/.. -A --ignore-errors
while (test -e .git/index.lock); do sleep 0.1; done
(echo ">  "compile NEMU"" && echo 231220000 张三 && uname -a && uptime) | git commit -F - -q --author='tracer-ics2024 <tracer@njuics.org>' --no-verify --allow-empty
sync
make -s -C nemu/tools/kvm-diff GUEST_ISA=x86 SHARE=1 ENGINE=interpreter
git add nemu/.. -A --ignore-errors
while (test -e .git/index.lock); do sleep 0.1; done
(echo ">  "run NEMU"" && echo 231220000 张三 && uname -a && uptime) | git commit -F - -q --author='tracer-ics2024 <tracer@njuics.org>' --no-verify --allow-empty
sync
nemu/build/x86-nemu-interpreter -l am-kernels/kernels/hello/build/nemu-log.txt --diff=nemu/tools/kvm-diff/build/x86-kvm-so am-kernels/kernels/hello/build/hello-x86-nemu.bin am-kernels/kernels/hello/build/hello-x86-nemu.elf
make[1]: Leaving directory 'nemu'
```
