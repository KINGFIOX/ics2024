#ifndef __INST_H__
#define __INST_H__

#include "common.h"
#include "cpu/decode.h"

void push(int width, word_t data);
word_t pop(int width);
void leave(void);

void callo(Decode* s, int w, word_t imm);
void calla(Decode* s, int w, word_t imm);
void ret(Decode* s, int w);

void cmp(int w, word_t op1, word_t op2);
void test(int w, word_t op1, word_t op2);
word_t add(int w, word_t op1, word_t op2, bool adc);
word_t sub(int w, word_t op1, word_t op2, bool sbb);
word_t and_(int w, word_t op1, word_t op2);
word_t xor_(int w, word_t op1, word_t op2);
word_t shr(int w, word_t op1, word_t op2);
word_t shl(int w, word_t op1, word_t op2);
word_t sar(int w, word_t op1, word_t op2);
word_t not_(int w, word_t op1);
word_t imul2(int w, word_t op1, word_t op2);
word_t or_(int w, word_t op1, word_t op2);

void jcc(Decode* s, word_t imm, uint8_t subcode);
void jmpo(Decode* s, word_t imm);
void jmpa(Decode* s, word_t imm);

word_t reg_read(int idx, int width);
void reg_write(int idx, int width, word_t data);

#define all ((word_t)(-1))

#define Rr reg_read
#define Rw reg_write

#define Mr vaddr_read

#define Mw vaddr_write

#endif