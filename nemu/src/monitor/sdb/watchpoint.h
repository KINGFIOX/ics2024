#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  bool valid;
  word_t last_value;
  char expr[64];

  /* TODO: Add more members if necessary */

} WP;

WP *new_wp();

#endif