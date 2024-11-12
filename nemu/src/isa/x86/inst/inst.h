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
void cmp_r_m(int rd, int w, word_t addr);
void cmp_r_i(int rd, int w, word_t imm);
void cmp_m_r(int rd, int w, word_t addr);
void cmpb(int w, word_t addr, word_t imm);
void test(int rd, int w, int rs);

word_t add(int w, word_t op1, word_t op2);

void je(Decode* s, word_t imm);
void jne(Decode* s, word_t imm);

word_t reg_read(int idx, int width);
void reg_write(int idx, int width, word_t data);

#define Rr reg_read
#define Rw reg_write
#define Mr vaddr_read
#define Mw vaddr_write

#endif