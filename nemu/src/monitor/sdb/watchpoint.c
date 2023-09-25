/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    memset(wp_pool[i].expr, 0, EXPR_LEN);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp(char* expr){
  WP* wp = free_;
  if (free_ == NULL){
    Log("Run out of watch points");
    return NULL;
  }

  wp->next = head;
  memset(wp->expr, 0, EXPR_LEN);
  strcpy(wp->expr, expr);
  head = wp;
  free_ = free_->next;
  return wp;
}

void free_wp(int index){
  WP** wp_ptr = &head;

  while (*wp_ptr){
    if ((*wp_ptr)->NO == index){
      (*wp_ptr)->next = free_;
      free_ = *wp_ptr;
      *wp_ptr = (*wp_ptr)->next;
      break;
    }
    wp_ptr = &(*wp_ptr)->next;
  } 
}


