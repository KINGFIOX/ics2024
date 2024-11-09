# report

## 1. 状态机

请看 ./sum100

## 2. 理解基础设施

## 3. RTFM

1. EFLAGS 寄存器中的 CF 位是什么意思

carry flag, 会表示: 无符号运算中, 发生了进位/借位, 实际上也就是 overflow

2. ModR/M 字节是什么

![the friendly manual is here](https://en.wikipedia.org/wiki/ModR/M)

里面的 displacement 表示: 偏移

3. mov 指令的具体格式是怎么样的

![the friendly manual is here](https://nju-projectn.github.io/i386-manual/MOV.htm)

## 4. shell 命令

1. 完成 PA1 的内容之后, nemu/目录下的所有.c 和.h 和文件总共有多少行代码?

| Language | files | size     | blank | comment | code   |
| -------- | ----- | -------- | ----- | ------- | ------ |
| C        | 251   | 2.54 MB  | 12939 | 7488    | 75477  |
| C Header | 555   | 6.41 MB  | 23861 | 47070   | 102390 |
| -        | -     | -        | -     | -       | -      |
| Sum      | 1398  | 12.72 MB | 41296 | 61300   | 270657 |

2. 你是使用什么命令得到这个结果的

cloc

3. 和框架代码相比, 你在 PA1 中编写了多少行代码?

不知道写了多少行, 这没法确定, 因为我开启了格式化工具, diff 比较大.
我 checkout 设置了一个 pa0 分支, 我可以通过 git diff pa0 pa1 来确定我修改了多少行

4. 再来个难一点的, 除去空行之外, nemu/目录下的所有.c 和.h 文件总共有多少行代码?

cloc 就可以去掉空行

## 5. CFLAGS -Werror -Wall

- `-Wall` enable all warnings
- `-Werror` treat warnings as errors

严格的检查, 避免一些潜在的错误
