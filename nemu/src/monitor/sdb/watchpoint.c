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

WP* new_wp(char* expression){
  WP* wp = free_;
  if (free_ == NULL){
    Log("Run out of watch points");
    return NULL;
  }
  free_ = free_->next;
  wp->next = head;
  strcpy(wp->expr, expression);
  head = wp;
  bool success;
  wp->prev_value = expr(wp->expr, &success);
  return wp;
}

void free_wp(int index){
  WP** wp_ptr = &head;

  while (*wp_ptr){
    if ((*wp_ptr)->NO == index){
      WP* temp = *wp_ptr;
      *wp_ptr = (*wp_ptr)->next;
      temp->next = free_;
      free_ = temp;
      break;
    }
    wp_ptr = &(*wp_ptr)->next;
  } 
}

void print_wp(){
  WP* wp = head;
  while (wp){
    printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
    wp = wp->next;
  }
}

bool check_change(){
  WP* wp = head;
  bool success;
  while (wp){
    word_t val = expr(wp->expr, &success);
    if(wp->prev_value != val){
      Log("Watchpoint %d: %s changed: Prev: %u, Now: %u", wp->NO, wp->expr, wp->prev_value, val);
      wp->prev_value = val;
      return true;
    }
    wp = wp->next;
  }
  return false;
}


