#ifndef __INST_H__
#define __INST_H__

#include "common.h"
#include "cpu/decode.h"

void push(int width, word_t data);
word_t pop(int width);
void leave(void);

void call(Decode* s, int w, word_t imm);
void ret(Decode* s, int w);

void cmp(int w, word_t op1, word_t op2);
void cmp_rm(int w, int rd, int rs, vaddr_t addr);
void test(int w, word_t op1, word_t op2);
word_t add(int w, word_t op1, word_t op2);
word_t sub(int w, word_t op1, word_t op2);
word_t and_(int w, word_t op1, word_t op2);
word_t xor_(int w, word_t op1, word_t op2);
word_t shr(int w, word_t op1, word_t op2);
word_t not_(int w, word_t op1);

void je(Decode* s, word_t imm);
void jne(Decode* s, word_t imm);
void jmp(Decode* s, word_t imm);
void jb(Decode* s, word_t imm);

word_t reg_read(int idx, int width);
void reg_write(int idx, int width, word_t data);

#define Rr reg_read
#define Rw reg_write
#define Mr vaddr_read
#define Mw vaddr_write

#endif