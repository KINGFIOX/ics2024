/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

#include "watchpoint.h"

static WP wp_pool[NR_WP] = {};  // 32 watchpoints at most
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].valid = false;
  }

  head = NULL;
  free_ = wp_pool;
}

WP *find_wp(int no) {
  if (0 <= no && no < NR_WP) {
    return &wp_pool[no];
  } else {
    return NULL;
  }
}

void watchpoint_display() {
  // TODO:
  printf("Num What\n");
  for (WP *cur = head; cur != NULL; cur = cur->next) {
    if (cur->valid) {
      printf("%d %s\n", cur->NO, cur->expr);
    }
  }
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp() {
  if (free_ == NULL) {
    assert(0);
  }
  WP *wp = free_;
  free_ = free_->next;
  wp->valid = true;
  wp->last_value = 0;
  memset(wp->expr, 0, sizeof(wp->expr));
  return wp;
}

void free_wp(WP *wp) {
  if (wp == NULL) {
    assert(0);
  }
  wp->next = free_;
  free_ = wp;
  wp->valid = false;
}
