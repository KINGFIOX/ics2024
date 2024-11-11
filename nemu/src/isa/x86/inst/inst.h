#ifndef __INST_H__
#define __INST_H__

#include "common.h"
#include "cpu/decode.h"

void push(int width, word_t data);
word_t pop(int width);

void call(Decode* s, int w, word_t imm);
void ret(Decode* s, int w);

word_t reg_read(int idx, int width);
void reg_write(int idx, int width, word_t data);

#endif