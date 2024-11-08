#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define WP_EXPR_LEN 64

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  bool valid;
  word_t last_value;
  char expr[WP_EXPR_LEN];

  /* TODO: Add more members if necessary */

} WP;

WP *new_wp(void);
WP *find_wp(int no);
void free_wp(WP *wp);
void watchpoint_display(void);
bool check_wp(void);

#endif