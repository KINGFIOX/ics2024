#ifndef __INST_H__
#define __INST_H__

#include "common.h"
#include "cpu/decode.h"
#include "memory/vaddr.h"

void push(int width, word_t data);
word_t pop(int width);
void leave(void);

void call(Decode* s, int w, word_t imm);
void ret(Decode* s, int w);
void cmp(int rd, int w, word_t addr);
void cmpb(int w, word_t addr, word_t imm);

void je(Decode* s, word_t imm);

word_t reg_read(int idx, int width);
void reg_write(int idx, int width, word_t data);

#define Rr reg_read
#define Rw reg_write
#define Mr vaddr_read
#define Mw vaddr_write

#endif